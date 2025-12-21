#include "renderer/Renderer.h"
#include "renderer/RenderAPI.h"
#include "renderer/VertexArray.h"

namespace CloudCore {

std::unique_ptr<RenderAPI> Renderer::s_RenderAPI = nullptr;
Renderer::State Renderer::s_State = {};

void Renderer::init() {
  s_RenderAPI->init();

  // Initialize state cache to match RenderAPI defaults
  s_State = State{};
}

void Renderer::shutdown() { s_RenderAPI->shutdown(); }

void Renderer::beginFrame() { s_RenderAPI->beginFrame(); }

void Renderer::endFrame() { s_RenderAPI->endFrame(); }

void Renderer::setViewport(uint32_t x, uint32_t y, uint32_t width,
                           uint32_t height) {
  s_RenderAPI->setViewport(x, y, width, height);
}

void Renderer::setClearColor(const Eigen::Vector4f &color) {
  if (s_State.clearColor == color) {
    return;
  }

  s_State.clearColor = color;
  s_RenderAPI->setClearColor(color);
}

void Renderer::clear() { s_RenderAPI->clear(); }

void Renderer::drawIndexed(const std::shared_ptr<VertexArray> &vertexArray,
                           uint32_t indexCount) {
  s_RenderAPI->drawIndexed(vertexArray, indexCount);
}

void Renderer::drawArrays(const std::shared_ptr<VertexArray> &vertexArray,
                          uint32_t vertexCount, PrimitiveType type) {
  s_RenderAPI->drawArrays(vertexArray, vertexCount, type);
}

void Renderer::setDepthTest(bool enabled) {
  if (s_State.depthTest == enabled) {
    return;
  }

  s_State.depthTest = enabled;
  s_RenderAPI->setDepthTest(enabled);
}

void Renderer::setDepthMask(bool enabled) {
  if (s_State.depthMask == enabled) {
    return;
  }

  s_State.depthMask = enabled;
  s_RenderAPI->setDepthMask(enabled);
}

void Renderer::setBlending(bool enabled) {
  if (s_State.blending == enabled) {
    return;
  }

  s_State.blending = enabled;
  s_RenderAPI->setBlending(enabled);
}

void Renderer::setWireframe(bool enabled) {
  if (s_State.wireframe == enabled) {
    return;
  }

  s_State.wireframe = enabled;
  s_RenderAPI->setWireframe(enabled);
}

void Renderer::setPointSize(float size) {
  if (s_State.pointSize == size) {
    return;
  }

  s_State.pointSize = size;
  s_RenderAPI->setPointSize(size);
}

void Renderer::setLineWidth(float size) {
  if (s_State.lineWidth == size) {
    return;
  }

  s_State.lineWidth = size;
  s_RenderAPI->setLineWidth(size);
}

void Renderer::setRenderAPI(std::unique_ptr<RenderAPI> renderAPI) {
  s_RenderAPI = std::move(renderAPI);
}

} // namespace CloudCore