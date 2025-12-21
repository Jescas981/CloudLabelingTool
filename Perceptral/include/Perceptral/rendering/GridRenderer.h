#pragma once

#include <Perceptral/renderer/Shader.h>
#include <Perceptral/renderer/VertexArray.h>
#include <Eigen/Core>
#include <memory>

namespace Perceptral {

struct GridSettings {
  // Grid type
  enum class Type {
    INFINITE_GRID, // Extends to horizon (most common)
    FINITE_GRID,   // Fixed size grid
    RADIAL_GRID,   // Circular grid
    CUSTOM_GRID    // Custom pattern
  } type = Type::INFINITE_GRID;

  // Appearance
  Eigen::Vector4f majorLineColor = {0.5f, 0.5f, 0.5f, 0.8f}; // Gray
  Eigen::Vector4f minorLineColor = {0.3f, 0.3f, 0.3f, 0.4f}; // Dark gray

  float majorLineWidth = 1.5f;
  float minorLineWidth = 0.5f;

  // Spacing
  float majorSpacing = 1.0f; // Every meter (or unit)
  float minorDivisions = 10; // 10 minor lines per major

  // Size and position
  float size = 100.0f;                         // For finite grid
  Eigen::Vector3f center = {0.0f, 0.0f, 0.0f}; // World position
  Eigen::Vector3f normal = {0.0f, 1.0f, 0.0f}; // Usually XZ plane (Y up)

  // Fade effects
  bool fadeEnabled = true;
  float fadeStartDistance = 10.0f;
  float fadeEndDistance = 50.0f;

  // Snap-to-grid functionality
  bool snapEnabled = true;
  float snapStrength = 0.2f; // 0-1, how strong the snap is

  // Visibility
  bool enabled = true;
  bool alwaysVisible = false; // Ignore frustum culling
};

class Shader;
class Camera;

class GridRenderer {
public:
  GridRenderer();
  ~GridRenderer();

  void initialize();

  void render(const Camera &camera);

private:
    std::size_t vertexCount_;
  std::shared_ptr<Shader> shader_;
  std::shared_ptr<VertexArray> vertexArray_;
  GridSettings settings_;

  void createSimpleGrid();
  // Cached data for optimization
  GridSettings lastSettings_;
  Eigen::Vector3f lastCameraPosition_;
  float lastGridScale_ = 1.0f;
  bool needsUpdate_ = true;

  // Constants
  static constexpr int MAX_GRID_LINES = 1000;
  static constexpr float UPDATE_THRESHOLD =
      0.1f; // Update when camera moves this far
};

} // namespace Perceptral