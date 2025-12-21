#include <Perceptral/core/Log.h>
#include <Perceptral/renderer/Buffer.h>
#include <Perceptral/renderer/Shader.h>
#include <Perceptral/rendering/AxisLineRenderer.h>
#include <glad/glad.h>
#include <Perceptral/core/Camera.h>

namespace Perceptral {

AxisLineRenderer::AxisLineRenderer() {}

AxisLineRenderer::~AxisLineRenderer() {}

void AxisLineRenderer::initialize() {
  vertexArray_ = VertexArray::create();
  shader_ = Shader::create("shaders/axislines.glsl");

  struct Vertex {
    float x, y, z;
    float r, g, b, a;
  };

  std::vector<Vertex> vertices;

  // Grid parameters
  const float gridSize = 100.0f;  // Total size of grid
  const int gridLines = 100;       // Number of lines in each direction
  const float gridStep = gridSize / gridLines;
  const float halfSize = gridSize / 2.0f;

  // Grid color (subtle gray)
  const float gridR = 0.3f, gridG = 0.3f, gridB = 0.3f;

  // Create grid on XY plane (Z=0, since Z is up)
  // Lines parallel to X axis (varying Y)
  for (int i = 0; i <= gridLines; ++i) {
    float y = -halfSize + i * gridStep;
    
    // Determine if this is a major grid line (every 10)
    bool isMajor = (i % 10 == 0);
    float alpha = isMajor ? 0.5f : 0.3f;
    
    // Skip the lines that coincide with axes (we'll draw them separately)
    if (std::abs(y) < 0.01f) continue;
    
    vertices.push_back({-halfSize, y, 0.0f, gridR, gridG, gridB, alpha});
    vertices.push_back({halfSize, y, 0.0f, gridR, gridG, gridB, alpha});
  }

  // Lines parallel to Y axis (varying X)
  for (int i = 0; i <= gridLines; ++i) {
    float x = -halfSize + i * gridStep;
    
    bool isMajor = (i % 10 == 0);
    float alpha = isMajor ? 0.5f : 0.3f;
    
    if (std::abs(x) < 0.01f) continue;
    
    vertices.push_back({x, -halfSize, 0.0f, gridR, gridG, gridB, alpha});
    vertices.push_back({x, halfSize, 0.0f, gridR, gridG, gridB, alpha});
  }

  gridVertexCount_ = vertices.size();

  // Now add the main axis lines (thicker, colored)
  const float axisLength = halfSize;

  // X axis (RED) - extends along X
  vertices.push_back({0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f});
  vertices.push_back({axisLength, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f});
  vertices.push_back({0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f});
  vertices.push_back({-axisLength, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f});

  // Y axis (GREEN) - extends along Y
  vertices.push_back({0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f});
  vertices.push_back({0.0f, axisLength, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f});
  vertices.push_back({0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f});
  vertices.push_back({0.0f, -axisLength, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f});

  // Z axis (BLUE) - extends along Z (up)
  vertices.push_back({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f});
  vertices.push_back({0.0f, 0.0f, axisLength, 0.0f, 0.0f, 1.0f, 1.0f});
  vertices.push_back({0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f});
  vertices.push_back({0.0f, 0.0f, -axisLength, 0.0f, 0.0f, 0.5f, 1.0f});

  totalVertexCount_ = vertices.size();

  auto vertexBuffer = VertexBuffer::create(
      vertices.data(), vertices.size() * sizeof(Vertex));
  vertexBuffer->setLayout({{ShaderDataType::Float3, "aPosition"},
                           {ShaderDataType::Float4, "aColor"}});

  vertexArray_->addVertexBuffer(vertexBuffer);

  PC_CORE_INFO("Grid and axis renderer initialized: {} grid lines, {} axis lines", 
               gridVertexCount_, totalVertexCount_ - gridVertexCount_);
}

void AxisLineRenderer::render(const Camera &camera) {
  if (!shader_ || !vertexArray_) {
    return;
  }

  // Save OpenGL state
  GLfloat prevLineWidth;
  glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
  GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
  GLboolean blendEnabled = glIsEnabled(GL_BLEND);
  GLint blendSrc, blendDst;
  glGetIntegerv(GL_BLEND_SRC_ALPHA, &blendSrc);
  glGetIntegerv(GL_BLEND_DST_ALPHA, &blendDst);

  // Enable blending for transparent grid
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);

  shader_->bind();
  shader_->setMat4("uViewProjection", camera.getViewProjectionMatrix());
  shader_->setMat4("uModel", Eigen::Matrix4f::Identity());
  
  vertexArray_->bind();

  // Draw grid lines (thin)
  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, gridVertexCount_);

  // Draw axis lines (thick)
  glLineWidth(3.0f);
  glDrawArrays(GL_LINES, gridVertexCount_, totalVertexCount_ - gridVertexCount_);

  vertexArray_->unbind();
  shader_->unbind();

  // Restore state
  glLineWidth(prevLineWidth);
  if (!depthTestEnabled) {
    glDisable(GL_DEPTH_TEST);
  }
  if (!blendEnabled) {
    glDisable(GL_BLEND);
  }
  glBlendFunc(blendSrc, blendDst);
}

} // namespace Perceptral