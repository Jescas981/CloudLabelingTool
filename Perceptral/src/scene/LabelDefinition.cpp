#include <Perceptral/core/Log.h>
#include <Perceptral/scene/LabelDefinition.h>

namespace Perceptral {

LabelDefinition::LabelDefinition() { loadDefaultLabels(); }

void LabelDefinition::addLabel(uint8_t id, const std::string &name,
                               const Eigen::Vector3f &color) {
  if (hasLabel(id)) {
    PC_CORE_WARN("Label with ID {} already exists, updating instead", id);
    updateLabel(id, name, color);
    return;
  }

  LabelClass label(id, name, color);
  labels_.push_back(label);
  labelIndexMap_[id] = labels_.size() - 1;

  PC_CORE_INFO("Added label: {} - {}", id, name);
}

void LabelDefinition::removeLabel(uint8_t id) {
  auto it = labelIndexMap_.find(id);
  if (it == labelIndexMap_.end()) {
    PC_CORE_WARN("Cannot remove label {}: not found", id);
    return;
  }

  size_t index = it->second;
  labels_.erase(labels_.begin() + index);
  labelIndexMap_.erase(it);

  // Rebuild index map
  labelIndexMap_.clear();
  for (size_t i = 0; i < labels_.size(); ++i) {
    labelIndexMap_[labels_[i].id] = i;
  }

  PC_CORE_INFO("Removed label ID {}", id);
}

void LabelDefinition::updateLabel(uint8_t id, const std::string &name,
                                  const Eigen::Vector3f &color) {
  auto *label = getLabel(id);
  if (!label) {
    PC_CORE_WARN("Cannot update label {}: not found", id);
    return;
  }

  label->name = name;
  label->color = color;
  PC_CORE_INFO("Updated label: {} - {}", id, name);
}

bool LabelDefinition::hasLabel(uint8_t id) const {
  return labelIndexMap_.find(id) != labelIndexMap_.end();
}

const LabelClass *LabelDefinition::getLabel(uint8_t id) const {
  auto it = labelIndexMap_.find(id);
  if (it == labelIndexMap_.end()) {
    return nullptr;
  }
  return &labels_[it->second];
}

LabelClass *LabelDefinition::getLabel(uint8_t id) {
  auto it = labelIndexMap_.find(id);
  if (it == labelIndexMap_.end()) {
    return nullptr;
  }
  return &labels_[it->second];
}

void LabelDefinition::clear() {
  labels_.clear();
  labelIndexMap_.clear();
}

void LabelDefinition::loadDefaultLabels() {
  clear();

  addLabel(0, "Unclassified",
           Eigen::Vector3f(0.6f, 0.6f, 0.6f));                  // Neutral gray
  addLabel(1, "Ground", Eigen::Vector3f(0.55f, 0.27f, 0.07f));  // Earth brown
  addLabel(2, "Vegetation", Eigen::Vector3f(0.0f, 0.6f, 0.0f)); // Strong green
  addLabel(3, "Buildings", Eigen::Vector3f(0.2f, 0.6f, 1.0f));  // Bright blue
  addLabel(4, "Poles", Eigen::Vector3f(1.0f, 0.0f, 0.0f));      // Red
  addLabel(5, "Wires", Eigen::Vector3f(1.0f, 0.4f, 0.4f)); // Light red / coral

  PC_CORE_INFO("Loaded default labels");
}

void LabelDefinition::loadLiDARLabels() {
  clear();

  addLabel(0, "Unclassified",
           Eigen::Vector3f(0.6f, 0.6f, 0.6f));                  // Neutral gray
  addLabel(1, "Ground", Eigen::Vector3f(0.55f, 0.27f, 0.07f));  // Earth brown
  addLabel(2, "Vegetation", Eigen::Vector3f(0.0f, 0.6f, 0.0f)); // Strong green
  addLabel(3, "Buildings", Eigen::Vector3f(0.2f, 0.6f, 1.0f));  // Bright blue
  addLabel(4, "Poles BT", Eigen::Vector3f(1.0f, 0.0f, 0.0f));   // Red
  addLabel(5, "Wires BT",
           Eigen::Vector3f(1.0f, 0.4f, 0.4f)); // Light red / coral
  addLabel(6, "Poles MT", Eigen::Vector3f(0.6f, 0.0f, 0.6f)); // Deep purple
  addLabel(7, "Wires MT", Eigen::Vector3f(1.0f, 0.0f, 1.0f)); // Magenta
  addLabel(8, "Ads",
           Eigen::Vector3f(1.0f, 1.0f, 0.0f)); // Yellow (very visible)
  addLabel(9, "Sand", Eigen::Vector3f(0.95f, 0.85f, 0.4f));    // Sand beige
  addLabel(10, "Andamios", Eigen::Vector3f(0.0f, 1.0f, 1.0f)); // Cyan
  addLabel(11, "Stairs", Eigen::Vector3f(0.3f, 0.3f, 0.3f));   // Dark gray
  addLabel(12, "Bricks", Eigen::Vector3f(0.8f, 0.2f, 0.0f));   // Brick red
  addLabel(13, "Works surround",
           Eigen::Vector3f(0.0f, 0.0f, 0.8f)); // Deep blue

  PC_CORE_INFO("Loaded LiDAR classification labels");
}

void LabelDefinition::loadSemanticSegmentationLabels() {
  clear();

  // Common semantic segmentation classes
  addLabel(0, "Unlabeled", Eigen::Vector3f(0.0f, 0.0f, 0.0f));
  addLabel(1, "Car", Eigen::Vector3f(0.0f, 0.0f, 1.0f));
  addLabel(2, "Bicycle", Eigen::Vector3f(1.0f, 0.0f, 1.0f));
  addLabel(3, "Motorcycle", Eigen::Vector3f(1.0f, 0.5f, 0.0f));
  addLabel(4, "Truck", Eigen::Vector3f(0.0f, 0.5f, 1.0f));
  addLabel(5, "Person", Eigen::Vector3f(1.0f, 0.0f, 0.0f));
  addLabel(6, "Road", Eigen::Vector3f(0.5f, 0.5f, 0.5f));
  addLabel(7, "Sidewalk", Eigen::Vector3f(0.8f, 0.8f, 0.0f));
  addLabel(8, "Building", Eigen::Vector3f(0.7f, 0.0f, 0.0f));
  addLabel(9, "Fence", Eigen::Vector3f(0.6f, 0.3f, 0.0f));
  addLabel(10, "Vegetation", Eigen::Vector3f(0.0f, 0.8f, 0.0f));
  addLabel(11, "Terrain", Eigen::Vector3f(0.55f, 0.27f, 0.07f));
  addLabel(12, "Pole", Eigen::Vector3f(1.0f, 1.0f, 0.0f));
  addLabel(13, "Traffic Sign", Eigen::Vector3f(1.0f, 1.0f, 1.0f));

  PC_CORE_INFO("Loaded semantic segmentation labels");
}

} // namespace Perceptral
