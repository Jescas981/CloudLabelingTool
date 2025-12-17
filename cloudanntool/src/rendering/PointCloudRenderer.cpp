#include "rendering/PointCloudRenderer.h"
#include "core/Camera.h"
#include "core/Log.h"

#include "renderer/Buffer.h"
#include "renderer/VertexArray.h"
#include "scene/Components.h"
#include "scene/PointCloud.h"
#include <glad/glad.h>

namespace CloudCore {

PointCloudRenderer::PointCloudRenderer()
    : pointSize_(3.0f), lastPointCount_(0), lastSelectionHash_(0) {}

PointCloudRenderer::~PointCloudRenderer() {}

bool PointCloudRenderer::initialize() {
  // Create shader
  shader_ = Shader::create("shaders/point.glsl");

  vertexArray_ = VertexArray::create();

  // Enable point rendering
  glEnable(GL_PROGRAM_POINT_SIZE);

  CC_CORE_INFO("PointCloudRenderer initialized");
  return true;
}

void PointCloudRenderer::render(const PointCloud &pointCloud,
                                const PointCloudComponent &component,
                                const Camera &camera) {
  if (pointCloud.empty()) {
    return;
  }

  pointSize_ = component.pointSize;

  // Compute simple hash of selection mask and visibility mask to detect changes
  size_t selectionHash = 0;
  if (!component.selectionMask.empty()) {
    // Simple hash: sum of all values + size
    for (auto val : component.selectionMask) {
      selectionHash += val;
    }
    selectionHash ^= component.selectionMask.size();
  }

  // Add visibility mask to hash
  if (!component.visibilityMask.empty()) {
    for (auto val : component.visibilityMask) {
      selectionHash +=
          val * 17; // Different multiplier to distinguish from selection
    }
    selectionHash ^= (component.visibilityMask.size() << 8);
  }

  // Update buffers if point count changed or selection/visibility mask changed
  bool needsUpdate = (pointCloud.size() != lastPointCount_);
  needsUpdate = needsUpdate || (selectionHash != lastSelectionHash_);

  if (needsUpdate) {
    CC_CORE_TRACE("Updating buffers: pointCount={}, selectionHash={} (prev={})",
                  pointCloud.size(), selectionHash, lastSelectionHash_);
    updateBuffers(pointCloud, component);
    lastPointCount_ = pointCloud.size();
    lastSelectionHash_ = selectionHash;
  }

  // Use shader
  shader_->bind();

  // Set matrices
  Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
  shader_->setMat4("uViewProjection", camera.getViewProjectionMatrix());
  shader_->setMat4("uModel", model);
  shader_->setFloat("uPointSize", pointSize_);

  // Set rendering mode - same for selected and unselected, but selected uses
  // selectionColor
  int colorMode = static_cast<int>(component.colorMode);
  shader_->setInt("uColorMode", colorMode); // FlatColor mode for selected points
  shader_->setFloat3("uSelectedColor", component.selectionColor);
  shader_->setFloat3("uUnselectedColor", component.flatColor);

  // Set label display mode and colors
  shader_->setBool("uShowLabels", component.showLabels);
  if (component.showLabels && component.labelDefinition) {
    // Upload label colors to shader
    const auto &labels = component.labelDefinition->getAllLabels();
    for (size_t i = 0; i < 256; ++i) {
      Eigen::Vector3f color(0.5f, 0.5f, 0.5f); // Default gray
      // Find label with this ID
      for (const auto &label : labels) {
        if (label.id == i) {
          color = label.color;
          break;
        }
      }
      std::string uniformName = "uLabelColors[" + std::to_string(i) + "]";
      shader_->setFloat3(uniformName, color);
    }
  }

  // Set bounding box for axis coloring and gradients
  shader_->setFloat3("uBoundsMin", pointCloud.getMin());
  shader_->setFloat3("uBoundsMax", pointCloud.getMax());

  // Render points
  vertexArray_->bind();
  glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(pointCloud.size()));

  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    CC_CORE_ERROR("OpenGL error after drawing: {}", err);
  }

  vertexArray_->unbind();
}

void PointCloudRenderer::updateBuffers(const PointCloud &pointCloud,
                                       const PointCloudComponent &component) {
  auto cloud = pointCloud.getCloud();

  // Prepare vertex data: position (vec3) + color (vec3) + selectionMask (float)
  // + label (float)
  std::vector<float> vertexData;
  vertexData.reserve(
      cloud->size() *
      8); // 3 for position, 3 for color, 1 for selection, 1 for label

  //   size_t visibleCount = 0;
  for (size_t i = 0; i < cloud->size(); ++i) {
    // Check if point is visible
    bool isVisible = true;
    if (!component.visibilityMask.empty() &&
        i < component.visibilityMask.size()) {
      isVisible = (component.visibilityMask[i] != 0);
    }

    // Skip hidden points
    if (!isVisible) {
      continue;
    }

    const auto &point = cloud->points[i];

    // Position
    vertexData.push_back(point.x);
    vertexData.push_back(point.y);
    vertexData.push_back(point.z);

    // Color - just use a default (shader will apply color mode)
    // The actual color will be determined by the shader based on colorMode
    vertexData.push_back(0.8f);
    vertexData.push_back(0.8f);
    vertexData.push_back(0.8f);

    // Selection mask (0.0 = not selected, 1.0 = selected)
    float selectionValue = 0.0f;
    if (!component.selectionMask.empty() &&
        i < component.selectionMask.size()) {
      selectionValue = (component.selectionMask[i] != 0) ? 1.0f : 0.0f;
    }
  
    vertexData.push_back(selectionValue);

    // Label ID (0-255)
    float labelValue = 0.0f;
    if (!component.labels.empty() && i < component.labels.size()) {
      labelValue = static_cast<float>(component.labels[i]);
    }
    vertexData.push_back(labelValue);

  }

  auto vertexBuffer = VertexBuffer::create(vertexData.data(),
                                           vertexData.size() * sizeof(float));

  vertexBuffer->setLayout({
      {ShaderDataType::Float3, "aPosition"},     // 3 floats for position
      {ShaderDataType::Float3, "aColor"},        // 3 floats for color
      {ShaderDataType::Float, "aSelectionMask"}, // 1 float for mask
      {ShaderDataType::Float, "aLabel"}          // 1 float for label
  });

  vertexArray_->clearVertexBuffers();
  vertexArray_->addVertexBuffer(vertexBuffer);

  vertexArray_->unbind();
}

} // namespace CloudCore
