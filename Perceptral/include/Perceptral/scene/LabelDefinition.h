#pragma once

#include <string>
#include <vector>
#include <map>
#include <Eigen/Core>

namespace Perceptral {

/**
 * @brief Defines a single label class for point cloud annotation
 */
struct LabelClass {
    uint8_t id;                      // Unique identifier (0-255)
    std::string name;                // Human-readable name
    Eigen::Vector3f color;           // RGB color for visualization (0-1 range)
    bool visible = true;             // Whether points with this label are visible

    LabelClass() : id(0), name("Unlabeled"), color(0.5f, 0.5f, 0.5f) {}
    LabelClass(uint8_t id, const std::string& name, const Eigen::Vector3f& color)
        : id(id), name(name), color(color) {}
};

/**
 * @brief Manages label definitions for point cloud classification
 */
class LabelDefinition {
public:
    LabelDefinition();
    ~LabelDefinition() = default;

    // Label management
    void addLabel(uint8_t id, const std::string& name, const Eigen::Vector3f& color);
    void removeLabel(uint8_t id);
    void updateLabel(uint8_t id, const std::string& name, const Eigen::Vector3f& color);

    // Queries
    bool hasLabel(uint8_t id) const;
    const LabelClass* getLabel(uint8_t id) const;
    LabelClass* getLabel(uint8_t id);
    const std::vector<LabelClass>& getAllLabels() const { return labels_; }

    // Preset configurations
    void loadDefaultLabels();
    void loadLiDARLabels();
    void loadSemanticSegmentationLabels();
    void clear();

private:
    std::vector<LabelClass> labels_;
    std::map<uint8_t, size_t> labelIndexMap_; // Maps ID to index in labels_ vector
};

} // namespace Perceptral
