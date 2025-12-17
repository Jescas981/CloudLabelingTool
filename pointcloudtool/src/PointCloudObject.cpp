#include "PointCloudObject.h"
#include "scene/Scene.h"  // Required for Entity template implementations
#include "core/Log.h"

namespace PointCloudTool {

PointCloudObject::PointCloudObject(const std::string& name, std::shared_ptr<CloudCore::PointCloud> pointCloud)
    : name_(name)
    , pointCloud_(pointCloud)
{
    CC_CORE_INFO("Created PointCloudObject: {}", name_);
}

void PointCloudObject::onCreate(CloudCore::Entity entity)
{
    entity_ = entity;

    // Add components to the entity
    auto& pcComp = entity_.addComponent<CloudCore::PointCloudComponent>(pointCloud_);
    entity_.addComponent<CloudCore::RenderableComponent>();

    // Set initial properties
    pcComp.visible = visible_;

    // Initialize color buffers
    initializeColors();

    CC_CORE_INFO("PointCloudObject '{}' created with {} points", name_, pointCloud_->size());
}

void PointCloudObject::onUpdate(CloudCore::Timestep deltaTime)
{
    // Object-specific update logic can go here
    // For example: animations, physics, procedural modifications, etc.
}

void PointCloudObject::onDestroy()
{
    CC_CORE_INFO("Destroying PointCloudObject: {}", name_);
    // Cleanup logic if needed
}

CloudCore::PointCloudComponent* PointCloudObject::getComponent()
{
    if (entity_) {
        return &entity_.getComponent<CloudCore::PointCloudComponent>();
    }
    return nullptr;
}

void PointCloudObject::setVisible(bool visible)
{
    visible_ = visible;
    if (auto* comp = getComponent()) {
        comp->visible = visible;
    }
}

void PointCloudObject::setSelected(bool selected)
{
    selected_ = selected;
    // Note: per-point selection is now handled via selectionMask
    // This method is kept for backwards compatibility but doesn't do anything
}

void PointCloudObject::setColorMode(CloudCore::PointCloudColorMode mode)
{
    if (auto* comp = getComponent()) {
        comp->colorMode = mode;
    }
}

void PointCloudObject::setPointSize(float size)
{
    if (auto* comp = getComponent()) {
        comp->pointSize = size;
    }
}

void PointCloudObject::setSingleColor(const Eigen::Vector3f& color)
{
    if (auto* comp = getComponent()) {
        comp->flatColor = color;
    }
}

void PointCloudObject::setSelectionColor(const Eigen::Vector3f& color)
{
    if (auto* comp = getComponent()) {
        comp->selectionColor = color;
    }
}

size_t PointCloudObject::getPointCount() const
{
    return pointCloud_ ? pointCloud_->size() : 0;
}

Eigen::Vector3f PointCloudObject::getCenter() const
{
    return pointCloud_ ? pointCloud_->getCenter() : Eigen::Vector3f::Zero();
}

float PointCloudObject::getBoundingSphereRadius() const
{
    return pointCloud_ ? pointCloud_->getBoundingSphereRadius() : 0.0f;
}

Eigen::Vector3f PointCloudObject::getMin() const
{
    return pointCloud_ ? pointCloud_->getMin() : Eigen::Vector3f::Zero();
}

Eigen::Vector3f PointCloudObject::getMax() const
{
    return pointCloud_ ? pointCloud_->getMax() : Eigen::Vector3f::Zero();
}

void PointCloudObject::selectPoints(const std::vector<size_t>& indices, bool additive)
{
    if (!additive) {
        selectedPoints_.clear();
    }

    // Add new selections (avoid duplicates)
    for (size_t idx : indices) {
        if (idx < getPointCount()) {
            if (std::find(selectedPoints_.begin(), selectedPoints_.end(), idx) == selectedPoints_.end()) {
                selectedPoints_.push_back(idx);
            }
        }
    }

    // Update selection mask
    updatePointColors();

    CC_CORE_INFO("Object '{}': {} points selected (total: {})",
                 name_, indices.size(), selectedPoints_.size());
}

void PointCloudObject::deselectPoints(const std::vector<size_t>& indices)
{
    // Remove specified points from selection
    for (size_t idx : indices) {
        auto it = std::find(selectedPoints_.begin(), selectedPoints_.end(), idx);
        if (it != selectedPoints_.end()) {
            selectedPoints_.erase(it);
        }
    }

    // Update selection mask
    updatePointColors();

    CC_CORE_INFO("Object '{}': {} points deselected (remaining: {})",
                 name_, indices.size(), selectedPoints_.size());
}

void PointCloudObject::clearSelection()
{
    selectedPoints_.clear();

    // Clear the selection mask
    if (auto* comp = getComponent()) {
        std::fill(comp->selectionMask.begin(), comp->selectionMask.end(), 0);
    }

    CC_CORE_INFO("Object '{}': Selection cleared", name_);
}

void PointCloudObject::initializeColors()
{
    if (!pointCloud_) return;

    size_t numPoints = pointCloud_->size();
    if (numPoints == 0) return;

    // Initialize with light gray color for all points
    Eigen::Vector3f defaultColor(0.8f, 0.8f, 0.8f);  // Light gray
    originalColors_.resize(numPoints, defaultColor);
    currentColors_.resize(numPoints, defaultColor);
    hasColorData_ = true;

    // Initialize selection mask (all unselected)
    if (auto* comp = getComponent()) {
        comp->selectionMask.resize(numPoints, 0);
    }

    // Initialize labels
    initializeLabels();

    CC_CORE_INFO("Initialized color buffers for {} points", numPoints);
}

void PointCloudObject::initializeLabels()
{
    if (!pointCloud_) return;

    auto* comp = getComponent();
    if (!comp) return;

    size_t numPoints = pointCloud_->size();

    // Create shared label definition if it doesn't exist
    if (!comp->labelDefinition) {
        comp->labelDefinition = std::make_shared<CloudCore::LabelDefinition>();
        comp->labelDefinition->loadDefaultLabels();
        CC_CORE_INFO("Created default label definition for '{}'", name_);
    }

    // Initialize all points as unclassified (label 0)
    comp->labels.resize(numPoints, 0);

    CC_CORE_INFO("Initialized labels for {} points", numPoints);
}

void PointCloudObject::assignLabelToSelected(uint8_t labelId, bool overwrite)
{
    auto* comp = getComponent();
    if (!comp || selectedPoints_.empty()) {
        CC_CORE_WARN("Cannot assign label: no component or no selected points");
        return;
    }

    // Ensure labels are initialized
    if (comp->labels.empty()) {
        initializeLabels();
    }

    // Assign label to selected points
    size_t assignedCount = 0;
    size_t skippedCount = 0;
    for (size_t idx : selectedPoints_) {
        if (idx < comp->labels.size()) {
            // If overwrite is false, only label points that are currently 0 (unclassified)
            if (!overwrite && comp->labels[idx] != 0) {
                skippedCount++;
                continue;
            }
            comp->labels[idx] = labelId;
            assignedCount++;
        }
    }

    const char* labelName = "Unknown";
    if (comp->labelDefinition) {
        const auto* label = comp->labelDefinition->getLabel(labelId);
        if (label) {
            labelName = label->name.c_str();
        }
    }

    if (!overwrite && skippedCount > 0) {
        CC_CORE_INFO("Assigned label {} ({}) to {} points, skipped {} points with existing labels",
                     labelId, labelName, assignedCount, skippedCount);
    } else {
        CC_CORE_INFO("Assigned label {} ({}) to {} selected points",
                     labelId, labelName, assignedCount);
    }
}

std::shared_ptr<CloudCore::LabelDefinition> PointCloudObject::getLabelDefinition()
{
    auto* comp = getComponent();
    if (!comp) return nullptr;

    if (!comp->labelDefinition) {
        initializeLabels();
    }

    return comp->labelDefinition;
}

void PointCloudObject::updatePointColors()
{
    if (!hasColorData_) return;

    // Update the selection mask based on selectedPoints_
    if (auto* comp = getComponent()) {
        size_t numPoints = pointCloud_->size();

        // IMPORTANT: Clear all values first, then set selected ones
        // resize() only sets new elements to 0, not existing ones!
        comp->selectionMask.assign(numPoints, 0);

        // Mark selected points in the mask
        for (size_t idx : selectedPoints_) {
            if (idx < comp->selectionMask.size()) {
                comp->selectionMask[idx] = 1;
            }
        }

        CC_CORE_TRACE("Updated selection mask: {} selected points", selectedPoints_.size());
    }

    // Mark that we have color data
    hasColorData_ = true;
}

// Visibility management
void PointCloudObject::hideLabel(uint8_t labelId)
{
    auto* comp = getComponent();
    if (!comp) return;

    // Add to hidden labels set
    comp->hiddenLabels.insert(labelId);

    // Update visibility mask
    size_t numPoints = pointCloud_->size();
    if (comp->visibilityMask.empty()) {
        comp->visibilityMask.assign(numPoints, 1); // Initialize all visible
    }

    // Hide all points with this label
    size_t hiddenCount = 0;
    for (size_t i = 0; i < numPoints && i < comp->labels.size(); ++i) {
        if (comp->labels[i] == labelId) {
            comp->visibilityMask[i] = 0;
            hiddenCount++;
        }
    }

    CC_CORE_INFO("Hidden label {} - {} points hidden", labelId, hiddenCount);
}

void PointCloudObject::showSelection()
{
    auto* comp = getComponent();
    if (!comp) return;
    
    size_t numPoints = pointCloud_->size();
    
    // Initialize visibility mask if empty
    if (comp->visibilityMask.empty()) {
        comp->visibilityMask.assign(numPoints, 0); // Start with all hidden
    }
    
    // Hide all points first
    comp->visibilityMask.assign(numPoints, 0);
    
    // Show only selected points
    if (!comp->selectionMask.empty()) {
        for (size_t i = 0; i < comp->selectionMask.size() && i < numPoints; ++i) {
            if (comp->selectionMask[i] != 0) {
                comp->visibilityMask[i] = 1; // Make selected points visible
            }
        }
    }
    
    CC_CORE_INFO("showSelection: {} points visible out of {}", 
                 selectedPoints_.size(), numPoints);
}

void PointCloudObject::showLabel(uint8_t labelId)
{
    auto* comp = getComponent();
    if (!comp) return;

    // Remove from hidden labels set
    comp->hiddenLabels.erase(labelId);

    // Update visibility mask
    size_t numPoints = pointCloud_->size();
    if (comp->visibilityMask.empty()) {
        comp->visibilityMask.assign(numPoints, 1); // Initialize all visible
    }

    // Show all points with this label
    size_t shownCount = 0;
    for (size_t i = 0; i < numPoints && i < comp->labels.size(); ++i) {
        if (comp->labels[i] == labelId) {
            comp->visibilityMask[i] = 1;
            shownCount++;
        }
    }

    CC_CORE_INFO("Shown label {} - {} points shown", labelId, shownCount);
}

void PointCloudObject::showAllLabels()
{
    auto* comp = getComponent();
    if (!comp) return;

    // Clear hidden labels set
    comp->hiddenLabels.clear();

    // Set all points to visible
    size_t numPoints = pointCloud_->size();
    comp->visibilityMask.assign(numPoints, 1);

    CC_CORE_INFO("All labels shown - {} points visible", numPoints);
}

void PointCloudObject::hideAllExceptLabel(uint8_t labelId)
{
    auto* comp = getComponent();
    if (!comp) return;

    // Clear and rebuild hidden labels set
    comp->hiddenLabels.clear();

    // Add all other labels to hidden set
    if (comp->labelDefinition) {
        const auto& labels = comp->labelDefinition->getAllLabels();
        for (const auto& label : labels) {
            if (label.id != labelId) {
                comp->hiddenLabels.insert(label.id);
            }
        }
    }

    // Update visibility mask
    size_t numPoints = pointCloud_->size();
    if (comp->visibilityMask.empty()) {
        comp->visibilityMask.assign(numPoints, 1);
    }

    size_t visibleCount = 0;
    for (size_t i = 0; i < numPoints && i < comp->labels.size(); ++i) {
        if (comp->labels[i] == labelId) {
            comp->visibilityMask[i] = 1;
            visibleCount++;
        } else {
            comp->visibilityMask[i] = 0;
        }
    }

    CC_CORE_INFO("Showing only label {} - {} points visible", labelId, visibleCount);
}

bool PointCloudObject::isLabelHidden(uint8_t labelId) const
{
    auto* comp = const_cast<PointCloudObject*>(this)->getComponent();
    if (!comp) return false;

    return comp->hiddenLabels.find(labelId) != comp->hiddenLabels.end();
}

} // namespace PointCloudTool
