#include "SelectionTool.h"
#include <Perceptral/core/Log.h>
#include <algorithm>
#include <thread>
#include <mutex>

namespace PointCloudTool {

void SelectionTool::startSelection(float x, float y)
{
    isSelecting_ = true;
    startX_ = x;
    startY_ = y;
    currentX_ = x;
    currentY_ = y;

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

    if (selectionMode_ == SelectionMode::Lasso) {
        if (!lassoPath_.empty()) {
            Eigen::Vector2f lastPoint = lassoPath_.back();
            float distSq = (x - lastPoint.x()) * (x - lastPoint.x()) + 
                          (y - lastPoint.y()) * (y - lastPoint.y());
            if (distSq > 4.0f) { // Squared distance > 2 pixels
                lassoPath_.push_back(Eigen::Vector2f(x, y));
            }
        }
    }
}

void SelectionTool::endSelection()
{
    if (!isSelecting_) return;
    isSelecting_ = false;

    if (selectionMode_ == SelectionMode::Lasso && lassoPath_.size() > 2) {
        lassoPath_.push_back(lassoPath_.front());
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

    auto cloud = pointCloud->getCloud();
    if (!cloud) {
        return {};
    }

    const size_t numPoints = cloud->size();
    
    if (selectionMode_ == SelectionMode::Rectangle) {
        return selectPointsRectangle(cloud, camera, windowWidth, windowHeight, numPoints);
    } else {
        return selectPointsLasso(cloud, camera, windowWidth, windowHeight, numPoints);
    }
}

std::vector<size_t> SelectionTool::selectPointsRectangle(
    pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
    Perceptral::Camera* camera,
    int windowWidth,
    int windowHeight,
    size_t numPoints) const
{
    float x1, y1, x2, y2;
    getSelectionRect(x1, y1, x2, y2);

    if (std::abs(x2 - x1) < 5.0f || std::abs(y2 - y1) < 5.0f) {
        PC_CORE_TRACE("Selection rectangle too small, ignoring");
        return {};
    }

    const Eigen::Matrix4f viewProjMatrix = camera->getViewProjectionMatrix();
    constexpr size_t PARALLEL_THRESHOLD = 50000;
    
    if (numPoints < PARALLEL_THRESHOLD) {
        return selectRectangleST(cloud, viewProjMatrix, x1, y1, x2, y2, 
                                 windowWidth, windowHeight, numPoints);
    } else {
        return selectRectangleMT(cloud, viewProjMatrix, x1, y1, x2, y2, 
                                 windowWidth, windowHeight, numPoints);
    }
}

std::vector<size_t> SelectionTool::selectRectangleST(
    pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
    const Eigen::Matrix4f& viewProjMatrix,
    float x1, float y1, float x2, float y2,
    int windowWidth, int windowHeight,
    size_t numPoints) const
{
    std::vector<size_t> selected;
    selected.reserve(numPoints / 100);

    const float halfWidth = windowWidth * 0.5f;
    const float halfHeight = windowHeight * 0.5f;

    for (size_t i = 0; i < numPoints; ++i) {
        const auto& p = cloud->points[i];
        
        // Transform to clip space
        Eigen::Vector4f clip = viewProjMatrix * Eigen::Vector4f(p.x, p.y, p.z, 1.0f);
        
        // Frustum culling
        if (clip.w() <= 1e-6f) continue;
        
        const float invW = 1.0f / clip.w();
        const float ndcX = clip.x() * invW;
        const float ndcY = clip.y() * invW;
        
        if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f) {
            continue;
        }
        
        // Convert to screen space
        const float screenX = (ndcX + 1.0f) * halfWidth;
        const float screenY = (1.0f - ndcY) * halfHeight;
        
        // Rectangle test
        if (screenX >= x1 && screenX <= x2 && screenY >= y1 && screenY <= y2) {
            selected.push_back(i);
        }
    }

    PC_CORE_INFO("Rectangle selection: {} points out of {}", selected.size(), numPoints);
    return selected;
}

std::vector<size_t> SelectionTool::selectRectangleMT(
    pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
    const Eigen::Matrix4f& viewProjMatrix,
    float x1, float y1, float x2, float y2,
    int windowWidth, int windowHeight,
    size_t numPoints) const
{
    const unsigned int numThreads = std::min(std::thread::hardware_concurrency(), 8u);
    const size_t chunkSize = (numPoints + numThreads - 1) / numThreads;
    
    std::vector<std::vector<size_t>> threadResults(numThreads);
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    
    const float halfWidth = windowWidth * 0.5f;
    const float halfHeight = windowHeight * 0.5f;
    
    auto worker = [&](size_t threadId) {
        std::vector<size_t> local;
        local.reserve(chunkSize / 100);
        
        const size_t start = threadId * chunkSize;
        const size_t end = std::min(start + chunkSize, numPoints);
        
        for (size_t i = start; i < end; ++i) {
            const auto& p = cloud->points[i];
            
            Eigen::Vector4f clip = viewProjMatrix * Eigen::Vector4f(p.x, p.y, p.z, 1.0f);
            
            if (clip.w() <= 1e-6f) continue;
            
            const float invW = 1.0f / clip.w();
            const float ndcX = clip.x() * invW;
            const float ndcY = clip.y() * invW;
            
            if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f) {
                continue;
            }
            
            const float screenX = (ndcX + 1.0f) * halfWidth;
            const float screenY = (1.0f - ndcY) * halfHeight;
            
            if (screenX >= x1 && screenX <= x2 && screenY >= y1 && screenY <= y2) {
                local.push_back(i);
            }
        }
        
        threadResults[threadId] = std::move(local);
    };
    
    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Merge results
    size_t totalSize = 0;
    for (const auto& result : threadResults) {
        totalSize += result.size();
    }
    
    std::vector<size_t> selected;
    selected.reserve(totalSize);
    
    for (auto& result : threadResults) {
        selected.insert(selected.end(), result.begin(), result.end());
    }
    
    PC_CORE_INFO("Rectangle selection (MT): {} points out of {}", selected.size(), numPoints);
    return selected;
}

std::vector<size_t> SelectionTool::selectPointsLasso(
    pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
    Perceptral::Camera* camera,
    int windowWidth,
    int windowHeight,
    size_t numPoints) const
{
    if (lassoPath_.size() < 3) {
        PC_CORE_TRACE("Lasso path too small, ignoring");
        return {};
    }

    // Compute lasso bounding box
    float minX = lassoPath_[0].x();
    float maxX = minX;
    float minY = lassoPath_[0].y();
    float maxY = minY;
    
    for (const auto& pt : lassoPath_) {
        minX = std::min(minX, pt.x());
        maxX = std::max(maxX, pt.x());
        minY = std::min(minY, pt.y());
        maxY = std::max(maxY, pt.y());
    }

    const Eigen::Matrix4f viewProjMatrix = camera->getViewProjectionMatrix();
    constexpr size_t PARALLEL_THRESHOLD = 50000;
    
    if (numPoints < PARALLEL_THRESHOLD) {
        return selectLassoST(cloud, viewProjMatrix, minX, maxX, minY, maxY,
                             windowWidth, windowHeight, numPoints);
    } else {
        return selectLassoMT(cloud, viewProjMatrix, minX, maxX, minY, maxY,
                             windowWidth, windowHeight, numPoints);
    }
}

std::vector<size_t> SelectionTool::selectLassoST(
    pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
    const Eigen::Matrix4f& viewProjMatrix,
    float minX, float maxX, float minY, float maxY,
    int windowWidth, int windowHeight,
    size_t numPoints) const
{
    std::vector<size_t> selected;
    selected.reserve(numPoints / 100);

    const float halfWidth = windowWidth * 0.5f;
    const float halfHeight = windowHeight * 0.5f;

    for (size_t i = 0; i < numPoints; ++i) {
        const auto& p = cloud->points[i];
        
        Eigen::Vector4f clip = viewProjMatrix * Eigen::Vector4f(p.x, p.y, p.z, 1.0f);
        
        if (clip.w() <= 1e-6f) continue;
        
        const float invW = 1.0f / clip.w();
        const float ndcX = clip.x() * invW;
        const float ndcY = clip.y() * invW;
        
        if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f) {
            continue;
        }
        
        const float screenX = (ndcX + 1.0f) * halfWidth;
        const float screenY = (1.0f - ndcY) * halfHeight;
        
        // Bounding box test first
        if (screenX < minX || screenX > maxX || screenY < minY || screenY > maxY) {
            continue;
        }
        
        // Expensive polygon test
        if (isPointInPolygonFast(screenX, screenY)) {
            selected.push_back(i);
        }
    }

    PC_CORE_INFO("Lasso selection: {} points out of {}", selected.size(), numPoints);
    return selected;
}

std::vector<size_t> SelectionTool::selectLassoMT(
    pcl::PointCloud<pcl::PointXYZL>::Ptr cloud,
    const Eigen::Matrix4f& viewProjMatrix,
    float minX, float maxX, float minY, float maxY,
    int windowWidth, int windowHeight,
    size_t numPoints) const
{
    const unsigned int numThreads = std::min(std::thread::hardware_concurrency(), 8u);
    const size_t chunkSize = (numPoints + numThreads - 1) / numThreads;
    
    std::vector<std::vector<size_t>> threadResults(numThreads);
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    
    const float halfWidth = windowWidth * 0.5f;
    const float halfHeight = windowHeight * 0.5f;
    
    auto worker = [&](size_t threadId) {
        std::vector<size_t> local;
        local.reserve(chunkSize / 100);
        
        const size_t start = threadId * chunkSize;
        const size_t end = std::min(start + chunkSize, numPoints);
        
        for (size_t i = start; i < end; ++i) {
            const auto& p = cloud->points[i];
            
            Eigen::Vector4f clip = viewProjMatrix * Eigen::Vector4f(p.x, p.y, p.z, 1.0f);
            
            if (clip.w() <= 1e-6f) continue;
            
            const float invW = 1.0f / clip.w();
            const float ndcX = clip.x() * invW;
            const float ndcY = clip.y() * invW;
            
            if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f) {
                continue;
            }
            
            const float screenX = (ndcX + 1.0f) * halfWidth;
            const float screenY = (1.0f - ndcY) * halfHeight;
            
            if (screenX < minX || screenX > maxX || screenY < minY || screenY > maxY) {
                continue;
            }
            
            if (isPointInPolygonFast(screenX, screenY)) {
                local.push_back(i);
            }
        }
        
        threadResults[threadId] = std::move(local);
    };
    
    for (unsigned int i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    size_t totalSize = 0;
    for (const auto& result : threadResults) {
        totalSize += result.size();
    }
    
    std::vector<size_t> selected;
    selected.reserve(totalSize);
    
    for (auto& result : threadResults) {
        selected.insert(selected.end(), result.begin(), result.end());
    }
    
    PC_CORE_INFO("Lasso selection (MT): {} points out of {}", selected.size(), numPoints);
    return selected;
}

bool SelectionTool::isPointInRect(const Eigen::Vector2f& screenPos, 
                                   float x1, float y1, float x2, float y2) const
{
    return screenPos.x() >= x1 && screenPos.x() <= x2 &&
           screenPos.y() >= y1 && screenPos.y() <= y2;
}

bool SelectionTool::isPointInPolygon(const Eigen::Vector2f& point, 
                                      const std::vector<Eigen::Vector2f>& polygon) const
{
    if (polygon.size() < 3) return false;

    bool inside = false;
    const float px = point.x();
    const float py = point.y();

    for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
        const float xi = polygon[i].x();
        const float yi = polygon[i].y();
        const float xj = polygon[j].x();
        const float yj = polygon[j].y();

        const bool intersect = ((yi > py) != (yj > py)) &&
                               (px < (xj - xi) * (py - yi) / (yj - yi) + xi);
        if (intersect) {
            inside = !inside;
        }
    }

    return inside;
}

bool SelectionTool::isPointInPolygonFast(float px, float py) const
{
    bool inside = false;
    const size_t n = lassoPath_.size();

    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        const float xi = lassoPath_[i].x();
        const float yi = lassoPath_[i].y();
        const float xj = lassoPath_[j].x();
        const float yj = lassoPath_[j].y();

        const bool intersect = ((yi > py) != (yj > py)) &&
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
    Eigen::Vector4f clipPos = camera->getViewProjectionMatrix() * 
                              Eigen::Vector4f(worldPos.x(), worldPos.y(), worldPos.z(), 1.0f);

    if (std::abs(clipPos.w()) > 1e-6f) {
        clipPos /= clipPos.w();
    }

    float screenX = (clipPos.x() + 1.0f) * 0.5f * windowWidth;
    float screenY = (1.0f - clipPos.y()) * 0.5f * windowHeight;

    return Eigen::Vector2f(screenX, screenY);
}

} // namespace PointCloudTool