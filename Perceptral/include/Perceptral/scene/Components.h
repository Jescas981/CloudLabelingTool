#pragma once

#include "PointCloud.h"
#include "LabelDefinition.h"
#include <Eigen/Core>
#include <string>
#include <memory>
#include <set>

namespace Perceptral {

// Tag component for identification
struct TagComponent {
    std::string tag;

    TagComponent() = default;
    TagComponent(const std::string& tag) : tag(tag) {}
};

// Transform component
struct TransformComponent {
    Eigen::Vector3f position{0.0f, 0.0f, 0.0f};
    Eigen::Vector3f rotation{0.0f, 0.0f, 0.0f}; // Euler angles
    Eigen::Vector3f scale{1.0f, 1.0f, 1.0f};

    TransformComponent() = default;
    TransformComponent(const Eigen::Vector3f& pos) : position(pos) {}

    Eigen::Matrix4f getTransform() const;
};

// Point cloud coloring modes
enum class PointCloudColorMode {
    RGB = 0,        // Use RGB colors from point cloud data
    FlatColor = 1,  // Use a single flat color
    AxisX = 2,      // Color based on X coordinate (gradient)
    AxisY = 3,      // Color based on Y coordinate (gradient)
    AxisZ = 4,      // Color based on Z coordinate (gradient)
    Gradient = 5    // Full gradient based on all axes
};

// Point cloud component
struct PointCloudComponent {
    std::shared_ptr<PointCloud> pointCloud;
    bool visible = true;
    float pointSize = 3.0f;

    // Simple color mode - applies to all points
    PointCloudColorMode colorMode = PointCloudColorMode::FlatColor;
    Eigen::Vector3f flatColor{0.8f, 0.8f, 0.8f}; // Used when colorMode = FlatColor

    // Labeling system - each point has a label ID
    std::vector<uint8_t> labels;  // Per-point label IDs (0-255)
    std::shared_ptr<LabelDefinition> labelDefinition; // Label definitions (shared across point clouds)
    bool showLabels = false;  // Whether to color points by label

    // Selection system with per-point mask (for editing/highlighting)
    std::vector<uint8_t> selectionMask;  // 0 = not selected, non-zero = selected
    Eigen::Vector3f selectionColor{1.0f, 0.5f, 0.0f}; // Orange - overrides everything for selected points

    // Visibility system - per-point visibility mask
    std::vector<uint8_t> visibilityMask;  // 0 = hidden, non-zero = visible (default: all visible)
    std::set<uint8_t> hiddenLabels;  // Set of label IDs that are currently hidden

    PointCloudComponent() = default;
    PointCloudComponent(std::shared_ptr<PointCloud> pc)
        : pointCloud(pc) {}
};

// Renderable component (marks entity for rendering)
struct RenderableComponent {
    bool castShadows = false;
    bool receiveShadows = false;
    int renderLayer = 0;

    RenderableComponent() = default;
};

// Camera component
struct CameraComponent {
    bool primary = true;
    bool fixedAspectRatio = false;

    CameraComponent() = default;
};

// Lifecycle component (for custom update logic)
struct ScriptComponent {
    // Function pointers for lifecycle
    std::function<void()> onCreate = nullptr;
    std::function<void(float)> onUpdate = nullptr;
    std::function<void()> onRender = nullptr;
    std::function<void()> onDestroy = nullptr;

    ScriptComponent() = default;
};

} // namespace Perceptral
