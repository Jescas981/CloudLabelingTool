#pragma once

#include <Perceptral/renderer/Shader.h>
#include <Perceptral/renderer/VertexArray.h>
#include <Eigen/Core>
#include <memory>

namespace Perceptral {

struct BackgroundSettings {
  // Gradient settings
  Eigen::Vector4f gradientTop = {0.0f, 0.0f, 0.0f, 1.0f};       // Black
  Eigen::Vector4f gradientBottom = {0.23f, 0.23f, 0.44f, 1.0f}; // Dark blue

  // Alternative: solid color
  Eigen::Vector4f clearColor = {0.1f, 0.1f, 0.1f, 1.0f};

  // Rendering mode
  enum class Mode {
    SOLID_COLOR,
    GRADIENT,
  } mode = Mode::GRADIENT;

  // Optional: enable/disable
  bool enabled = true;
};

class Shader;

class BackgroundRenderer {
public:
  BackgroundRenderer();
  ~BackgroundRenderer();

  void initialize(BackgroundSettings settings = BackgroundSettings{});

  void render();

private:
  std::shared_ptr<Shader> shader_;
  std::shared_ptr<VertexArray> vertexArray_;
  BackgroundSettings settings_;
};

} // namespace Perceptral