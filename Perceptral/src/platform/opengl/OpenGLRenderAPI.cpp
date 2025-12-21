#include <Perceptral/platform/opengl/OpenGLRenderAPI.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/renderer/VertexArray.h>
#include <glad/glad.h>
#include <iostream>

namespace Perceptral {

void OpenGLRenderAPI::init() {
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  PC_CORE_DEBUG("OpenGL Renderer initialized");
  PC_CORE_DEBUG("  Vendor: {}",
                reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  PC_CORE_DEBUG("  Renderer: {}",
                reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  PC_CORE_DEBUG("  Version: {}",
                reinterpret_cast<const char *>(glGetString(GL_VERSION)));
}

void OpenGLRenderAPI::shutdown() {
  // Cleanup if needed
}

void OpenGLRenderAPI::beginFrame() {
  // Nothing needed for OpenGL
}

void OpenGLRenderAPI::endFrame() {
  // Nothing needed for OpenGL
}

void OpenGLRenderAPI::setViewport(uint32_t x, uint32_t y, uint32_t width,
                                  uint32_t height) {
  glViewport(x, y, width, height);
}

void OpenGLRenderAPI::setClearColor(const Eigen::Vector4f &color) {
  glClearColor(color.x(), color.y(), color.z(), color.w());
}

void OpenGLRenderAPI::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderAPI::drawIndexed(
    const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount) {
  vertexArray->bind();
  uint32_t count =
      indexCount ? indexCount : vertexArray->getIndexBuffer()->getCount();
  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void OpenGLRenderAPI::drawArrays(
    const std::shared_ptr<VertexArray> &vertexArray, uint32_t vertexCount,
    PrimitiveType type) {
  vertexArray->bind();
  GLenum glType = GL_TRIANGLES; // Initialize with default
  switch (type) {
  case PrimitiveType::Points:
    glType = GL_POINTS;
    break;
  case PrimitiveType::Lines:
    glType = GL_LINES;
    break;
  case PrimitiveType::LineStrip:
    glType = GL_LINE_STRIP;
    break;
  case PrimitiveType::Triangles:
    glType = GL_TRIANGLES;
    break;
  case PrimitiveType::TriangleStrip:
    glType = GL_TRIANGLE_STRIP;
    break;
  case PrimitiveType::TriangleFan:
    glType = GL_TRIANGLE_FAN;
    break;
  }

  glDrawArrays(glType, 0, vertexCount);
}

void OpenGLRenderAPI::setDepthTest(bool enabled) {
  if (enabled) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void OpenGLRenderAPI::setDepthMask(bool enabled) {
  glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void OpenGLRenderAPI::setBlending(bool enabled) {
  if (enabled) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }
}

void OpenGLRenderAPI::setWireframe(bool enabled) {
  glPolygonMode(GL_FRONT_AND_BACK, enabled ? GL_LINE : GL_FILL);
}

void OpenGLRenderAPI::setPointSize(float size) { glPointSize(size); }

void OpenGLRenderAPI::setLineWidth(float size) { glLineWidth(size); }

} // namespace Perceptral