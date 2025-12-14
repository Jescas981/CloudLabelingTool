#include "core/Log.h"
#include <iostream>
#include "renderer/Buffer.h"
#include "renderer/Shader.h"
#include "rendering/AxisLineRenderer.h"
#include <glad/glad.h>
#include "core/Camera.h"

namespace CloudCore {

AxisLineRenderer::AxisLineRenderer() {}

AxisLineRenderer::~AxisLineRenderer() {}

void AxisLineRenderer::initialize() {
  vertexArray_ = VertexArray::create();
  shader_ = Shader::create("shaders/axislines.glsl");

  // Create axis lines: X (red), Y (green), Z (blue)
  struct AxisVertex {
    float x, y, z;    // position
    float r, g, b, a; // color
  };

  std::vector<AxisVertex> vertices = {
      // X axis (red) - from origin to +X
      {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
      {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},

      // Y axis (green) - from origin to +Y
      {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
      {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},

      // Z axis (blue) - from origin to +Z
      {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
      {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}};


  auto vertexBuffer = VertexBuffer::create(
      vertices.data(), vertices.size() * sizeof(AxisVertex));
  vertexBuffer->setLayout({{ShaderDataType::Float3, "aPosition"},
                           {ShaderDataType::Float4, "aColor"}});

  vertexArray_->addVertexBuffer(vertexBuffer);

  CC_CORE_INFO("Axis renderer initialized with 6 vertices (3 lines)");
}

void AxisLineRenderer::render(const Camera &camera) {
  if (!shader_ || !vertexArray_) {
    return;
  }

  // Save OpenGL state
  GLfloat prevLineWidth;
  glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
  GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

  // Set state for axis lines
  glEnable(GL_DEPTH_TEST);
  glLineWidth(2.0f); // Thicker lines for visibility

  // Bind shader and set uniforms
  shader_->bind();
  shader_->setMat4("uViewProjection", camera.getViewProjectionMatrix());
  shader_->setMat4("uModel", Eigen::Matrix4f::Identity());
  // Draw axes
  vertexArray_->bind();
  glDrawArrays(GL_LINES, 0, 6);
  vertexArray_->unbind();

  shader_->unbind();

  // Restore state
  glLineWidth(prevLineWidth);
  if (!depthTestEnabled) {
    glDisable(GL_DEPTH_TEST);
  }
}
} // namespace CloudCore