#include <Perceptral/rendering/BackgroundRenderer.h>
#include <Perceptral/core/Camera.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/renderer/Buffer.h>
#include <Perceptral/renderer/RenderAPI.h>
#include <Perceptral/renderer/VertexArray.h>
#include <Perceptral/renderer/Renderer.h>
#include <Perceptral/rendering/BackgroundRenderer.h>

namespace Perceptral {

BackgroundRenderer::BackgroundRenderer()
    : shader_(nullptr), vertexArray_(nullptr), settings_() {}

BackgroundRenderer::~BackgroundRenderer() {
  // Cleanup if needed
}

void BackgroundRenderer::initialize(BackgroundSettings settings) {
  settings_ = settings;
  shader_ = Shader::create("shaders/background.glsl");
  vertexArray_ = VertexArray::create();
  float vertices[12] = {
      0.0f, 0.0f, 0.0f, // bottom-left
      1.0f, 0.0f, 0.0f, // bottom-right
      0.0f, 1.0f, 0.0f, // top-left
      1.0f, 1.0f, 0.0f  // top-right
  };

  auto vertexBuffer = VertexBuffer::create(vertices, sizeof(vertices));
  BufferLayout layout{{ShaderDataType::Float3, "aPosition"}};
  vertexBuffer->setLayout(layout);
  vertexArray_->addVertexBuffer(vertexBuffer);
  vertexArray_->unbind();
}

void BackgroundRenderer::render() {
  if (!settings_.enabled) {
    return;
  }

  // Save current OpenGL state
  Renderer::setDepthTest(false);
  Renderer::setBlending(false);

  // Use shader
  shader_->bind();

  // Set uniforms based on mode
  shader_->setInt("uMode", static_cast<int>(settings_.mode));

  switch (settings_.mode) {
  case BackgroundSettings::Mode::SOLID_COLOR:
    shader_->setFloat4("uSolidColor", settings_.clearColor);
    break;

  case BackgroundSettings::Mode::GRADIENT:
    shader_->setFloat4("uGradientTop", settings_.gradientTop);
    shader_->setFloat4("uGradientBottom", settings_.gradientBottom);
    break;
  }

  vertexArray_->bind();
  Renderer::drawArrays(vertexArray_, 4, PrimitiveType::TriangleStrip);
  vertexArray_->unbind();

  Renderer::setDepthTest(true);
  Renderer::setBlending(true);


}

} // namespace Perceptral