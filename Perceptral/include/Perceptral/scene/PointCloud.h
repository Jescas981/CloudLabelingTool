#pragma once

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <string>
#include <Eigen/Core>

namespace Perceptral {

// Wrapper class for PCL point clouds with XYZRGB points
class PointCloud {
public:
    using PCLPointType = pcl::PointXYZL;
    using PCLCloudType = pcl::PointCloud<PCLPointType>;
    using PCLCloudPtr = PCLCloudType::Ptr;

    PointCloud();
    explicit PointCloud(PCLCloudPtr cloud);
    ~PointCloud();

    // Data management
    void addPoint(const PCLPointType& point);
    void addPoint(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b);
    void clear();
    void reserve(size_t size);

    // Getters
    PCLCloudPtr getCloud() { return cloud_; }
    const PCLCloudPtr getCloud() const { return cloud_; }
    size_t size() const { return cloud_->size(); }
    bool empty() const { return cloud_->empty(); }

    // Bounds
    void computeBounds();
    const Eigen::Vector3f& getMin() const { return boundsMin_; }
    const Eigen::Vector3f& getMax() const { return boundsMax_; }
    Eigen::Vector3f getCenter() const { return (boundsMin_ + boundsMax_) * 0.5f; }
    float getBoundingSphereRadius() const;

    // Transformations
    void translate(const Eigen::Vector3f& offset);
    void scale(float factor);
    void transform(const Eigen::Matrix4f& matrix);

    // Metadata
    inline void setName(const std::string& name) { name_ = name; }
    inline void setHasLabels(bool hasLabels) { hasLabels_ = hasLabels; }
    const std::string& getName() const { return name_; }
    bool getHasLabels() const { return hasLabels_; }

    // Direct PCL cloud access
    void setCloud(PCLCloudPtr cloud);

private:
    PCLCloudPtr cloud_;
    std::string name_;

    Eigen::Vector3f boundsMin_;
    Eigen::Vector3f boundsMax_;
    bool boundsDirty_;
    bool hasLabels_;
};

} // namespace Perceptral
