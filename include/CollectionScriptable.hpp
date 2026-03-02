#pragma once
#include "Perceptral/scene/Entity.h"
#include <Perceptral/Perceptral.h>
#include <Perceptral/core/assets/PointCloud.h>
#include <Perceptral/scene/Components.h>
#include <imgui.h>

struct CloudEntry {
  Perceptral::Entity entity;
  std::string name;
  bool visible{true};
  bool selected{false};
  std::vector<uint32_t> selectedPoints{};
};

class CollectionScriptable : public Perceptral::Scriptable {
public:
  void onCreate() override {}

  void addPointCloud(const std::string &filepath,
                     const std::vector<std::string> &fields) {
    Perceptral::Material material{"Cloud"};
    material.setColor(Eigen::Vector3f{0.5f, 0.5f, 0.5f});
    auto shader = Perceptral::AssetManager::load<Perceptral::Shader>(
        "core:pointcloud.glsl");
    material.setShader(shader);

    auto pointcloud =
        Perceptral::Asset::PointCloud::createFromFile(filepath, fields);
    auto entity = createChild();

    entity.addComponent<Perceptral::Component::PointCloudData>(pointcloud);
    entity.addComponent<Perceptral::Component::PointCloudRenderer>(material);

    // Extract filename as display name
    std::string name = std::filesystem::path(filepath).stem().string();
    clouds_.push_back({entity, name, true, false});
  }

  void setVisible(int index, bool visible) {
    if (!isValid(index))
      return;
    clouds_[index].visible = visible;
    // Toggle renderer component
    auto &e = clouds_[index].entity;
    e.getComponent<Perceptral::Component::PointCloudRenderer>().visible =
        visible;
  }

  void select(int index, bool additive = false) {
    if (!isValid(index))
      return;
    if (!additive)
      for (auto &c : clouds_)
        c.selected = false;
    clouds_[index].selected = !clouds_[index].selected;
    selectedIndex_ = index;
  }

  void remove(int index) {
    if (!isValid(index))
      return;
    // destroyEntity(clouds_[index].entity);
    clouds_.erase(clouds_.begin() + index);
    selectedIndex_ = std::min(selectedIndex_, (int)clouds_.size() - 1);
  }

  void applySelection(int cloudIndex) {
    if (!isValid(cloudIndex))
      return;

    auto &cloud = clouds_[cloudIndex];
    auto &cloudData =
        cloud.entity.getComponent<Perceptral::Component::PointCloudData>();

    const std::size_t pointCount = cloudData.asset->size();

    // Resize and clear
    cloudData.selected.assign(pointCount, false);

    // Mark selected indices
    for (uint32_t idx : cloud.selectedPoints)
      if (idx < pointCount)
        cloudData.selected[idx] = true;

    cloudData.isDirty = true; // triggers GPU re-upload in RenderSystem
  }

  void onImgGuiRender() override {
    ImGui::Begin("Scene");

    // Header
    ImGui::Text("Point Clouds (%zu)", getAll().size());
    ImGui::Separator();

    // Cloud list
    for (int i = 0; i < (int)getAll().size(); i++) {
      auto &cloud = getAll()[i];

      ImGui::PushID(i);

      // Visibility eye icon
      bool vis = cloud.visible;
      if (ImGui::Checkbox("##vis", &vis))
        setVisible(i, vis);

      ImGui::SameLine();

      // Selectable row — highlight if selected
      if (ImGui::Selectable(cloud.name.c_str(), cloud.selected,
                            ImGuiSelectableFlags_SpanAllColumns)) {
        select(i, ImGui::GetIO().KeyCtrl);
      }

      // Right-click context menu per cloud
      if (ImGui::BeginPopupContextItem("##ctx")) {
        if (ImGui::MenuItem("Rename")) { /* TODO */
        }
        if (ImGui::MenuItem("Focus")) { /* TODO: orbit camera to this cloud */
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete")) {
          remove(i);
          ImGui::EndPopup();
          ImGui::PopID();
          break; // list mutated, stop iterating
        }
        ImGui::EndPopup();
      }

      ImGui::PopID();
    }

    // Empty state hint
    if (getAll().empty()) {
      ImGui::TextDisabled("No clouds loaded.");
      ImGui::TextDisabled("File > Import to add one.");
    }

    ImGui::End();
  }

  const std::vector<CloudEntry> &getAll() const { return clouds_; }

  int getSelectedIndex() const { return selectedIndex_; }
  CloudEntry *getSelected() {
    return isValid(selectedIndex_) ? &clouds_[selectedIndex_] : nullptr;
  }

private:
  bool isValid(int i) const { return i >= 0 && i < (int)clouds_.size(); }

  std::vector<CloudEntry> clouds_;
  int selectedIndex_{-1};
};