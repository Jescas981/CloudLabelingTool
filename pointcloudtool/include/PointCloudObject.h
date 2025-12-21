#pragma once

#include "scene/Entity.h"
#include "scene/Components.h"
#include "scene/PointCloud.h"
#include "core/DeltaTime.h"
#include <memory>
#include <string>

namespace PointCloudTool {

/**
 * @brief Represents a point cloud object in the scene with its own logic and lifecycle
 *
 * Each PointCloudObject manages:
 * - The underlying point cloud data
 * - The ECS entity
 * - Visual properties (color mode, point size, etc.)
 * - Selection state
 * - Object-specific logic and updates
 */
class PointCloudObject {
public:
    PointCloudObject(const std::string& name, std::shared_ptr<CloudCore::PointCloud> pointCloud);
    ~PointCloudObject() = default;

    // Lifecycle methods
    void onCreate(CloudCore::Entity entity);
    void onUpdate(CloudCore::DeltaTime deltaTime);
    void onDestroy();

    // Getters
    const std::string& getName() const { return name_; }
    CloudCore::Entity getEntity() const { return entity_; }
    std::shared_ptr<CloudCore::PointCloud> getPointCloud() const { return pointCloud_; }
    CloudCore::PointCloudComponent* getComponent();

    bool isVisible() const { return visible_; }
    bool isSelected() const { return selected_; }

    // Setters
    void setVisible(bool visible);
    void setSelected(bool selected);
    void setColorMode(CloudCore::PointCloudColorMode mode);
    void setPointSize(float size);
    void setSingleColor(const Eigen::Vector3f& color);
    void setSelectionColor(const Eigen::Vector3f& color);

    // Statistics
    size_t getPointCount() const;
    Eigen::Vector3f getCenter() const;
    float getBoundingSphereRadius() const;
    Eigen::Vector3f getMin() const;
    Eigen::Vector3f getMax() const;

    // Point selection
    void selectPoints(const std::vector<size_t>& indices, bool additive = false);
    void deselectPoints(const std::vector<size_t>& indices);
    void clearSelection();
    const std::vector<size_t>& getSelectedPoints() const { return selectedPoints_; }
    bool hasSelection() const { return !selectedPoints_.empty(); }
    size_t getSelectionCount() const { return selectedPoints_.size(); }
    const Eigen::Vector3f& getSelectionColor() const { return pointSelectionColor_; }

    // Color access for rendering
    bool hasColors() const { return hasColorData_; }
    const std::vector<Eigen::Vector3f>& getColors() const { return currentColors_; }

    // Label management
    void assignLabelToSelected(uint8_t labelId, bool overwrite = true);
    void initializeLabels();
    std::shared_ptr<CloudCore::LabelDefinition> getLabelDefinition();

    // Visibility management
    void hideLabel(uint8_t labelId);
    void showLabel(uint8_t labelId);
    void showSelection();
    void showAllLabels();
    void hideAllExceptLabel(uint8_t labelId);
    bool isLabelHidden(uint8_t labelId) const;

private:
    std::string name_;
    std::shared_ptr<CloudCore::PointCloud> pointCloud_;
    CloudCore::Entity entity_;

    bool visible_ = true;
    bool selected_ = false;

    // Point selection tracking
    std::vector<size_t> selectedPoints_;
    Eigen::Vector3f pointSelectionColor_{1.0f, 1.0f, 0.0f}; // Yellow by default

    // Color management for selection
    std::vector<Eigen::Vector3f> originalColors_;
    std::vector<Eigen::Vector3f> currentColors_;
    bool hasColorData_ = false;

    // Helper methods
    void initializeColors();
    void updatePointColors();
};

} // namespace PointCloudTool
