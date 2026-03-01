#pragma once

#include <Eigen/Dense>
#include <Perceptral/core/Camera.h>
#include <Perceptral/scene/PointCloud.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <vector>
#include <memory>

namespace PointCloudTool {

enum class SelectionMode {
    Rectangle,
    Lasso
};

class SelectionTool {
public:
    SelectionTool() = default;
    ~SelectionTool() = default;

    void startSelection(float x, float y);
    void updateSelection(float x, float y);
    void endSelection();
    void cancelSelection();

    void setSelectionMode(SelectionMode mode) { selectionMode_ = mode; }
    SelectionMode getSelectionMode() const { return selectionMode_; }

    bool isSelecting() const { return isSelecting_; }
    void getSelectionRect(float& x1, float& y1, float& x2, float& y2) const;
    const std::vector<Eigen::Vector2f>& getLassoPath() const { return lassoPath_; }

    std::vector<size_t> selectPoints(
        std::shared_ptr<Perceptral::PointCloud> pointCloud,
        Perceptral::Camera* camera,
        int windowWidth,
        int windowHeight,
        bool additive = false) const;

private:
    // Main selection dispatch
    std::vector<size_t> selectPointsRectangle(
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
        Perceptral::Camera* camera,
        int windowWidth,
        int windowHeight,
        size_t numPoints) const;

    std::vector<size_t> selectPointsLasso(
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
        Perceptral::Camera* camera,
        int windowWidth,
        int windowHeight,
        size_t numPoints) const;

    // Rectangle selection implementations
    std::vector<size_t> selectRectangleST(
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
        const Eigen::Matrix4f& viewProjMatrix,
        float x1, float y1, float x2, float y2,
        int windowWidth, int windowHeight,
        size_t numPoints) const;

    std::vector<size_t> selectRectangleMT(
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
        const Eigen::Matrix4f& viewProjMatrix,
        float x1, float y1, float x2, float y2,
        int windowWidth, int windowHeight,
        size_t numPoints) const;

    // Lasso selection implementations
    std::vector<size_t> selectLassoST(
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
        const Eigen::Matrix4f& viewProjMatrix,
        float minX, float maxX, float minY, float maxY,
        int windowWidth, int windowHeight,
        size_t numPoints) const;

    std::vector<size_t> selectLassoMT(
        pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
        const Eigen::Matrix4f& viewProjMatrix,
        float minX, float maxX, float minY, float maxY,
        int windowWidth, int windowHeight,
        size_t numPoints) const;

    // Utility functions
    bool isPointInRect(const Eigen::Vector2f& screenPos, 
                      float x1, float y1, float x2, float y2) const;
    
    bool isPointInPolygon(const Eigen::Vector2f& point, 
                         const std::vector<Eigen::Vector2f>& polygon) const;
    
    bool isPointInPolygonFast(float px, float py) const;

    Eigen::Vector2f projectToScreen(
        const Eigen::Vector3f& worldPos,
        Perceptral::Camera* camera,
        int windowWidth,
        int windowHeight) const;

private:
    SelectionMode selectionMode_ = SelectionMode::Rectangle;
    bool isSelecting_ = false;

    // Rectangle selection state
    float startX_ = 0.0f;
    float startY_ = 0.0f;
    float currentX_ = 0.0f;
    float currentY_ = 0.0f;

    // Lasso selection state
    std::vector<Eigen::Vector2f> lassoPath_;
};

} // namespace PointCloudTool