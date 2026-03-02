#pragma once
#include "CollectionScriptable.hpp"
#include <Perceptral/Perceptral.h>
#include <imgui.h>
#include <toml++/toml.hpp>

struct LabelDef {
  std::string name{"Unlabeled"};
  int id{0};
  Eigen::Vector3f color{1.0f, 1.0f, 1.0f};

  // For ImGui input persistence
  char nameBuf[128]{};
  char idBuf[16]{};

  LabelDef(const std::string &name, int id, Eigen::Vector3f color)
      : name(name), id(id), color(color) {
    updateBuffers();
  }

  LabelDef() { updateBuffers(); }

  void updateBuffers() {
    std::strncpy(nameBuf, name.c_str(), sizeof(nameBuf) - 1);
    nameBuf[sizeof(nameBuf) - 1] = '\0'; // Ensure null termination
    std::snprintf(idBuf, sizeof(idBuf), "%d", id);
  }
};

class LabelingToolScriptable : public Perceptral::Scriptable {
public:
  void onCreate() override {
    auto &ns = getEntityByTag("Collection")
                   .getComponent<Perceptral::Component::NativeScript>();

    m_collection = Perceptral::GetScript<CollectionScriptable>(ns);

    // Default labels
    m_labels.push_back({"Ground", 0, {0.45f, 0.76f, 0.40f}});
    m_labels.push_back({"Vegetation", 1, {0.20f, 0.60f, 0.20f}});
    m_labels.push_back({"Building", 2, {0.80f, 0.40f, 0.20f}});
    m_labels.push_back({"Unclassified", 3, {0.70f, 0.70f, 0.70f}});
  }

  void onImgGuiRender() override {
    if (!m_panelOpen)
      return;

    ImGui::Begin("Labeling", &m_panelOpen);

    renderLabelList();
    ImGui::Separator();
    renderQuickAssignPanel();
    ImGui::Separator();
    renderViewSection();
    ImGui::End();
    renderManageModal();
  }

  // ─── MenuBar API ─────────────────────────────────────────────────────────
  bool isPanelOpen() const { return m_panelOpen; }
  void setPanelOpen(bool open) { m_panelOpen = open; }

  // ─── SelectionTool / MenuBar can call directly ────────────────────────────
  void applyLabel(int labelId) {
    auto *cloud = m_collection->getSelected();
    if (!cloud || cloud->selectedPoints.empty())
      return;

    auto &cloudData =
        cloud->entity.getComponent<Perceptral::Component::PointCloudData>();
    if (!cloudData.asset)
      return;

    auto &labels = cloudData.asset->labels;
    if (labels.size() < cloudData.asset->size())
      labels.resize(cloudData.asset->size(), 0);

    // Find the label definition for this ID
    const LabelDef *targetLabel = nullptr;
    for (const auto &label : m_labels) {
      if (label.id == labelId) {
        targetLabel = &label;
        break;
      }
    }

    if (!targetLabel)
      return; // Label not found

    // Update both labels and colors for selected points
    for (uint32_t idx : cloud->selectedPoints) {
      if (idx < labels.size()) {
        labels[idx] = static_cast<int32_t>(labelId);
      }
    }

    // Clear selection after applying
    cloud->selectedPoints.clear();
    m_collection->applySelection(m_collection->getSelectedIndex());

    // Make sure color mode is set to use the colors
    cloudData.isDirty = true;
  }

  void addLabel(const LabelDef &def) {
    m_labels.push_back(def);
    m_labels.back().updateBuffers(); // Add this line
  }

  void removeLabel(int i) { m_labels.erase(m_labels.begin() + i); }
  LabelDef &getLabel(int i) { return m_labels[i]; }
  const std::vector<LabelDef> &getLabels() const { return m_labels; }

private:
  void renderQuickAssignPanel() {
    ImGui::Text("Quick Assign:");
    ImGui::Separator();

    const int buttonSize = 24;
    const int columns = 2; // grid columns
    int count = 0;

    for (auto &label : m_labels) {
      ImGui::PushID(label.id);

      ImGui::PushStyleColor(
          ImGuiCol_Button,
          ImVec4(label.color[0], label.color[1], label.color[2], 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                            ImVec4(label.color[0] + 0.2f, label.color[1] + 0.2f,
                                   label.color[2] + 0.2f, 1.0f));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                            ImVec4(label.color[0] - 0.1f, label.color[1] - 0.1f,
                                   label.color[2] - 0.1f, 1.0f));

      if (ImGui::Button(label.name.c_str(), ImVec2(120, 0))) {
        applyLabel(label.id);
      }

      if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s (ID: %d)", label.name.c_str(), label.id);

      ImGui::PopStyleColor(3);
      ImGui::PopID();

      count++;
      if (count % columns != 0)
        ImGui::SameLine();
    }

    ImGui::NewLine();
  }

  // ─── Panel sections ───────────────────────────────────────────────────────
  void renderLabelList() {
    ImGui::Text("Labels");
    ImGui::SameLine();
    if (ImGui::SmallButton("Manage"))
      m_showManageModal = true;

    ImGui::Separator();

    for (auto &label : m_labels) {
      ImGui::PushID(label.id);

      // Color dot
      ImGui::ColorButton(
          "##dot", ImVec4(label.color[0], label.color[1], label.color[2], 1.0f),
          ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
      ImGui::SameLine();

      // Selectable — click to apply
      if (ImGui::Selectable(label.name.c_str(), m_activeLabel == label.id,
                            ImGuiSelectableFlags_None, ImVec2(0, 0)))
        m_activeLabel = label.id;

      ImGui::PopID();
    }
  }

  void renderApplySection() {
    auto *cloud = m_collection->getSelected();
    int count = cloud ? (int)cloud->selectedPoints.size() : 0;

    ImGui::Text("Selected: %d points", count);

    ImGui::BeginDisabled(count == 0 || m_activeLabel < 0);
    if (ImGui::Button("Apply Label", ImVec2(-1, 0)))
      applyLabel(m_activeLabel);
    ImGui::EndDisabled();
  }

  void renderViewSection() {
    auto *cloud = m_collection->getSelected();
    if (!cloud) {
      ImGui::TextDisabled("No cloud selected");
      return;
    }

    auto &cd =
        cloud->entity.getComponent<Perceptral::Component::PointCloudData>();

    ImGui::Text("Color Mode");

    // Dropdown for color mode
    const char *colorModeNames[] = {"Flat",   "Axis X",       "Axis Y",
                                    "Axis Z", "Scalar Field", "Label Field"};

    Perceptral::Component::ColorMode modes[] = {
        Perceptral::Component::ColorMode::FlatColor,
        Perceptral::Component::ColorMode::AxisColorX,
        Perceptral::Component::ColorMode::AxisColorY,
        Perceptral::Component::ColorMode::AxisColorZ,
        Perceptral::Component::ColorMode::ScalarField,
        Perceptral::Component::ColorMode::LabelField};

    // Find current selection index
    int currentIndex = 0;
    for (int i = 0; i < 6; i++)
      if (cd.colorMode == modes[i])
        currentIndex = i;

    if (ImGui::BeginCombo("##colormode", colorModeNames[currentIndex])) {
      for (int i = 0; i < 6; i++) {
        bool isSelected = (currentIndex == i);
        if (ImGui::Selectable(colorModeNames[i], isSelected)) {
          cd.colorMode = modes[i];
          cd.isDirty = true;
          currentIndex = i;
        }
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    // Scalar field selection (if needed)
    if (cd.colorMode == Perceptral::Component::ColorMode::ScalarField &&
        cd.asset) {
      ImGui::Separator();
      ImGui::Text("Field:");

      for (auto &[name, _] : cd.asset->fields) {
        bool active = cd.activeField == name;
        if (ImGui::RadioButton(name.c_str(), active)) {
          cd.activeField = name;
          cd.isDirty = true;
        }
      }
    }

    ImGui::Separator();
    ImGui::Text("Point Size");
    if (ImGui::SliderFloat("##ptsize", &cd.pointSize, 1.0f, 10.0f))
      cd.isDirty = true;
  }

  // ─── Manage modal ─────────────────────────────────────────────────────────
  void renderManageModal() {
    if (!m_showManageModal)
      return;

    ImGui::OpenPopup("Manage Labels");
    if (!ImGui::BeginPopupModal("Manage Labels", &m_showManageModal,
                                ImGuiWindowFlags_AlwaysAutoResize))
      return;

    int toDelete = -1;

    for (int i = 0; i < (int)m_labels.size(); i++) {
      auto &label = m_labels[i];
      ImGui::PushID(i);

      // Color editor
      ImGui::ColorEdit3("##c", label.color.data(),
                        ImGuiColorEditFlags_NoInputs |
                            ImGuiColorEditFlags_NoLabel);
      ImGui::SameLine();

      // Name editor
      ImGui::SetNextItemWidth(140.0f);
      if (ImGui::InputText("##n", label.nameBuf, sizeof(label.nameBuf))) {
        // Sync typed buffer to label
        label.name = label.nameBuf;
      }
      ImGui::SameLine();

      // ID editor
      ImGui::SetNextItemWidth(60.0f);
      if (ImGui::InputText("##id", label.idBuf, sizeof(label.idBuf),
                           ImGuiInputTextFlags_CharsDecimal)) {
        // Try converting to int
        try {
          int newID = std::stoi(label.idBuf);

          // Check for duplicates
          bool duplicate = false;
          for (auto &l : m_labels)
            if (&l != &label && l.id == newID)
              duplicate = true;

          if (!duplicate) {
            label.id = newID;
          }

          // Always sync buffer to actual ID
          std::snprintf(label.idBuf, sizeof(label.idBuf), "%d", label.id);

        } catch (...) {
          // Invalid input: revert buffer to current label.id
          std::snprintf(label.idBuf, sizeof(label.idBuf), "%d", label.id);
        }
      }
      ImGui::SameLine();

      // Delete button
      if (ImGui::SmallButton("X"))
        toDelete = i;

      ImGui::PopID();
    }

    if (toDelete >= 0)
      m_labels.erase(m_labels.begin() + toDelete);

    ImGui::Separator();

    // Add new label
    if (ImGui::Button("+ Add")) {
      m_labels.push_back(
          {"New Label", (int)m_labels.size(), Eigen::Vector3f{1, 1, 1}});
      m_labels.back().updateBuffers(); // Initialize buffers for new label
    }

    ImGui::SameLine();
    if (ImGui::Button("Close"))
      m_showManageModal = false;

    ImGui::EndPopup();
  }

private:
  CollectionScriptable *m_collection{nullptr};
  std::vector<LabelDef> m_labels;
  int m_activeLabel{-1};
  bool m_panelOpen{false};
  bool m_showManageModal{false};
};