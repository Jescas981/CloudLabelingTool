#include <Perceptral/scene/PointCloud.h>
#include <pcl/common/transforms.h>
#include <limits>
#include <cmath>

namespace Perceptral {

PointCloud::PointCloud()
    : cloud_(new PCLCloudType())
    , boundsDirty_(true)
    , hasLabels_(false)
{
    boundsMin_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max());
    boundsMax_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest());
}

PointCloud::PointCloud(PCLCloudPtr cloud)
    : cloud_(cloud)
    , boundsDirty_(true)
{
    if (!cloud_) {
        cloud_ = PCLCloudPtr(new PCLCloudType());
    }
    boundsMin_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max());
    boundsMax_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest());
}

PointCloud::~PointCloud()
{
}

void PointCloud::addPoint(const PCLPointType& point)
{
    cloud_->push_back(point);
    boundsDirty_ = true;
}

void PointCloud::addPoint(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b)
{
    // Note: r, g, b parameters are ignored since we're using PointXYZ (no color)
    // Color is applied via uniform in the shader
    (void)r; (void)g; (void)b; // Suppress unused parameter warnings

    PCLPointType point;
    point.x = x;
    point.y = y;
    point.z = z;
    cloud_->push_back(point);
    boundsDirty_ = true;
}

void PointCloud::clear()
{
    cloud_->clear();
    boundsDirty_ = true;
}

void PointCloud::reserve(size_t size)
{
    cloud_->reserve(size);
}

void PointCloud::setCloud(PCLCloudPtr cloud)
{
    cloud_ = cloud;
    if (!cloud_) {
        cloud_ = PCLCloudPtr(new PCLCloudType());
    }
    boundsDirty_ = true;
}

void PointCloud::computeBounds()
{
    if (!boundsDirty_ || cloud_->empty()) {
        return;
    }

    boundsMin_ = Eigen::Vector3f(std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max(),
                                  std::numeric_limits<float>::max());
    boundsMax_ = Eigen::Vector3f(std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest(),
                                  std::numeric_limits<float>::lowest());

    for (const auto& point : cloud_->points) {
        boundsMin_.x() = std::min(boundsMin_.x(), point.x);
        boundsMin_.y() = std::min(boundsMin_.y(), point.y);
        boundsMin_.z() = std::min(boundsMin_.z(), point.z);

        boundsMax_.x() = std::max(boundsMax_.x(), point.x);
        boundsMax_.y() = std::max(boundsMax_.y(), point.y);
        boundsMax_.z() = std::max(boundsMax_.z(), point.z);
    }

    boundsDirty_ = false;
}

float PointCloud::getBoundingSphereRadius() const
{
    if (cloud_->empty()) {
        return 0.0f;
    }

    Eigen::Vector3f center = getCenter();
    float maxDistSq = 0.0f;

    for (const auto& point : cloud_->points) {
        Eigen::Vector3f pos(point.x, point.y, point.z);
        float distSq = (pos - center).squaredNorm();
        if (distSq > maxDistSq) {
            maxDistSq = distSq;
        }
    }

    return std::sqrt(maxDistSq);
}

void PointCloud::translate(const Eigen::Vector3f& offset)
{
    for (auto& point : cloud_->points) {
        point.x += offset.x();
        point.y += offset.y();
        point.z += offset.z();
    }
    boundsDirty_ = true;
}

void PointCloud::scale(float factor)
{
    for (auto& point : cloud_->points) {
        point.x *= factor;
        point.y *= factor;
        point.z *= factor;
    }
    boundsDirty_ = true;
}

void PointCloud::transform(const Eigen::Matrix4f& matrix)
{
    PCLCloudPtr transformed(new PCLCloudType());
    pcl::transformPointCloud(*cloud_, *transformed, matrix);
    cloud_ = transformed;
    boundsDirty_ = true;
}

} // namespace Perceptral
