#include "SelectionTool.h"
#include <Perceptral/core/Log.h>
#include <algorithm>

namespace PointCloudTool {

void SelectionTool::startSelection(float x, float y)
{
    isSelecting_ = true;
    startX_ = x;
    startY_ = y;
    currentX_ = x;
    currentY_ = y;

    // For lasso mode, initialize the path
    if (selectionMode_ == SelectionMode::Lasso) {
        lassoPath_.clear();
        lassoPath_.push_back(Eigen::Vector2f(x, y));
    }

    PC_CORE_TRACE("Selection started at ({}, {})", x, y);
}

void SelectionTool::updateSelection(float x, float y)
{
    if (!isSelecting_) return;
    currentX_ = x;
    currentY_ = y;

    // For lasso mode, add point to path
    if (selectionMode_ == SelectionMode::Lasso) {
        // Only add if point is far enough from last point (avoid duplicate points)
        if (!lassoPath_.empty()) {
            Eigen::Vector2f lastPoint = lassoPath_.back();
            float distance = (Eigen::Vector2f(x, y) - lastPoint).norm();
            if (distance > 2.0f) { // Minimum 2 pixels distance
                lassoPath_.push_back(Eigen::Vector2f(x, y));
            }
        }
    }
}

void SelectionTool::endSelection()
{
    if (!isSelecting_) return;
    isSelecting_ = false;

    // For lasso mode, close the path
    if (selectionMode_ == SelectionMode::Lasso && !lassoPath_.empty()) {
        // Close the polygon by adding the first point at the end
        if (lassoPath_.size() > 2) {
            lassoPath_.push_back(lassoPath_.front());
        }
    }

    PC_CORE_TRACE("Selection ended at ({}, {})", currentX_, currentY_);
}

void SelectionTool::cancelSelection()
{
    isSelecting_ = false;
    lassoPath_.clear();
    PC_CORE_TRACE("Selection cancelled");
}

void SelectionTool::getSelectionRect(float& x1, float& y1, float& x2, float& y2) const
{
    x1 = std::min(startX_, currentX_);
    y1 = std::min(startY_, currentY_);
    x2 = std::max(startX_, currentX_);
    y2 = std::max(startY_, currentY_);
}

std::vector<size_t> SelectionTool::selectPoints(
    std::shared_ptr<Perceptral::PointCloud> pointCloud,
    Perceptral::Camera* camera,
    int windowWidth,
    int windowHeight,
    bool additive) const
{
    if (!pointCloud || pointCloud->empty()) {
        return {};
    }

    std::vector<size_t> selectedIndices;
    auto cloud = pointCloud->getCloud();
    if (!cloud) {
        return selectedIndices;
    }

    if (selectionMode_ == SelectionMode::Rectangle) {
        // Rectangle selection
        float x1, y1, x2, y2;
        getSelectionRect(x1, y1, x2, y2);

        // Ensure minimum rectangle size (avoid accidental clicks)
        if (std::abs(x2 - x1) < 5.0f || std::abs(y2 - y1) < 5.0f) {
            PC_CORE_TRACE("Selection rectangle too small, ignoring");
            return selectedIndices;
        }

        for (size_t i = 0; i < cloud->size(); ++i) {
            const auto& pclPoint = cloud->points[i];
            Eigen::Vector3f point(pclPoint.x, pclPoint.y, pclPoint.z);
            Eigen::Vector2f screenPos = projectToScreen(point, camera, windowWidth, windowHeight);

            if (isPointInRect(screenPos, x1, y1, x2, y2)) {
                selectedIndices.push_back(i);
            }
        }
    } else {
        // Lasso selection
        if (lassoPath_.size() < 3) {
            PC_CORE_TRACE("Lasso path too small, ignoring");
            return selectedIndices;
        }

        for (size_t i = 0; i < cloud->size(); ++i) {
            const auto& pclPoint = cloud->points[i];
            Eigen::Vector3f point(pclPoint.x, pclPoint.y, pclPoint.z);
            Eigen::Vector2f screenPos = projectToScreen(point, camera, windowWidth, windowHeight);

            if (isPointInPolygon(screenPos, lassoPath_)) {
                selectedIndices.push_back(i);
            }
        }
    }

    PC_CORE_INFO("{} selection: {} points out of {}",
                 (selectionMode_ == SelectionMode::Rectangle ? "Rectangle" : "Lasso"),
                 selectedIndices.size(), cloud->size());
    return selectedIndices;
}

bool SelectionTool::isPointInRect(const Eigen::Vector2f& screenPos, float x1, float y1, float x2, float y2) const
{
    return screenPos.x() >= x1 && screenPos.x() <= x2 &&
           screenPos.y() >= y1 && screenPos.y() <= y2;
}

bool SelectionTool::isPointInPolygon(const Eigen::Vector2f& point, const std::vector<Eigen::Vector2f>& polygon) const
{
    // Ray casting algorithm: count how many times a ray from the point crosses the polygon boundary
    // If odd number of crossings, point is inside
    if (polygon.size() < 3) return false;

    bool inside = false;
    float px = point.x();
    float py = point.y();

    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        float xi = polygon[i].x();
        float yi = polygon[i].y();
        float xj = polygon[j].x();
        float yj = polygon[j].y();

        // Check if ray crosses this edge
        bool intersect = ((yi > py) != (yj > py)) &&
                        (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}

Eigen::Vector2f SelectionTool::projectToScreen(
    const Eigen::Vector3f& worldPos,
    Perceptral::Camera* camera,
    int windowWidth,
    int windowHeight) const
{
    // Transform to clip space
    Eigen::Vector4f clipPos = camera->getViewProjectionMatrix() * Eigen::Vector4f(worldPos.x(), worldPos.y(), worldPos.z(), 1.0f);

    // Perspective divide
    if (std::abs(clipPos.w()) > 1e-6f) {
        clipPos /= clipPos.w();
    }

    // Convert from NDC (-1 to 1) to screen space (0 to width/height)
    float screenX = (clipPos.x() + 1.0f) * 0.5f * windowWidth;
    float screenY = (1.0f - clipPos.y()) * 0.5f * windowHeight; // Flip Y for screen coordinates

    return Eigen::Vector2f(screenX, screenY);
}

} // namespace PointCloudTool
