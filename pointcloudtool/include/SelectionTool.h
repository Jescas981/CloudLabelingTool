#pragma once

#include <Perceptral/core/Camera.h>
#include <Perceptral/scene/PointCloud.h>
#include <Eigen/Dense>
#include <vector>

namespace PointCloudTool {

enum class SelectionMode {
    Rectangle,  // Rectangle selection
    Lasso       // Freeform lasso selection
};

/**
 * @brief Selection tool for selecting points in a point cloud
 *
 * Supports both rectangle and lasso selection modes.
 * Uses screen-space to world-space projection for selection.
 */
class SelectionTool {
public:
    SelectionTool() = default;
    ~SelectionTool() = default;

    // Selection mode
    void setSelectionMode(SelectionMode mode) { selectionMode_ = mode; }
    SelectionMode getSelectionMode() const { return selectionMode_; }

    // Selection state
    void startSelection(float x, float y);
    void updateSelection(float x, float y);
    void endSelection();
    void cancelSelection();

    bool isSelecting() const { return isSelecting_; }

    // Get selection rectangle in screen coordinates (for rectangle mode)
    void getSelectionRect(float& x1, float& y1, float& x2, float& y2) const;

    // Get lasso path (for lasso mode)
    const std::vector<Eigen::Vector2f>& getLassoPath() const { return lassoPath_; }

    // Perform selection on a point cloud
    std::vector<size_t> selectPoints(
        std::shared_ptr<Perceptral::PointCloud> pointCloud,
        Perceptral::Camera* camera,
        int windowWidth,
        int windowHeight,
        bool additive = false  // If true, add to existing selection
    ) const;

private:
    SelectionMode selectionMode_ = SelectionMode::Rectangle;
    bool isSelecting_ = false;

    // Rectangle selection data
    float startX_ = 0.0f;
    float startY_ = 0.0f;
    float currentX_ = 0.0f;
    float currentY_ = 0.0f;

    // Lasso selection data (path of points)
    std::vector<Eigen::Vector2f> lassoPath_;

    // Helper: Check if a point is inside the selection rectangle (screen space)
    bool isPointInRect(const Eigen::Vector2f& screenPos, float x1, float y1, float x2, float y2) const;

    // Helper: Check if a point is inside a polygon using ray casting algorithm
    bool isPointInPolygon(const Eigen::Vector2f& point, const std::vector<Eigen::Vector2f>& polygon) const;

    // Helper: Project 3D point to screen space
    Eigen::Vector2f projectToScreen(
        const Eigen::Vector3f& worldPos,
        Perceptral::Camera* camera,
        int windowWidth,
        int windowHeight
    ) const;
};

} // namespace PointCloudTool
