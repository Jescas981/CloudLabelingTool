#pragma once
#include "CollectionScriptable.hpp"
#include "LabelingToolScriptable.hpp"
#include "Utils.h"
#include <Perceptral/Perceptral.h>
#include <imgui.h>

class MenuBarScriptable : public Perceptral::Scriptable {
public:
  void onCreate() override {
    auto &ns = getEntityByTag("Collection")
                   .getComponent<Perceptral::Component::NativeScript>();

    m_collection = Perceptral::GetScript<CollectionScriptable>(ns);

    auto &ns1 = getEntityByTag("LabelingTool")
                    .getComponent<Perceptral::Component::NativeScript>();

    m_labeling = Perceptral::GetScript<LabelingToolScriptable>(ns1);

    PointCloudTool::initOpenFileDialog();
  }

  void onImgGuiRender() override {
    renderMenu();
    renderFieldSelectorModal();
  }

private:
  void renderMenu() {
    if (!ImGui::BeginMainMenuBar())
      return;

    // File
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Import")) {
        std::string path =
            PointCloudTool::openFileDialog("Point Cloud Files", "ply");
        if (!path.empty()) {
          m_pendingFilePath = path;
          m_availableFields =
              Perceptral::Asset::PointCloud::getFieldsFromFile(path);
          m_selectedFields.assign(m_availableFields.size(), false);
          m_showFieldSelector = true;
        }
      }
      if (ImGui::MenuItem("Export")) {
      }
      if (ImGui::MenuItem("Exit")) {
      }
      ImGui::EndMenu();
    }

    // View
    if (ImGui::BeginMenu("View")) {
      bool panelOpen = m_labeling->isPanelOpen();
      if (ImGui::MenuItem("Labeling Panel", nullptr, panelOpen))
        m_labeling->setPanelOpen(!panelOpen);
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  void renderFieldSelectorModal() {
    if (!m_showFieldSelector)
      return;

    ImGui::OpenPopup("Select Fields");
    if (!ImGui::BeginPopupModal("Select Fields", nullptr,
                                ImGuiWindowFlags_AlwaysAutoResize))
      return;

    ImGui::Text("XYZ will always be loaded.");
    ImGui::Separator();
    for (size_t i = 0; i < m_availableFields.size(); ++i) {
      bool b = m_selectedFields[i];
      if (ImGui::Checkbox(m_availableFields[i].c_str(), &b))
        m_selectedFields[i] = b;
    }

    ImGui::Separator();
    if (ImGui::Button("Load")) {
      std::vector<std::string> chosen;
      for (size_t i = 0; i < m_availableFields.size(); ++i)
        if (m_selectedFields[i])
          chosen.push_back(m_availableFields[i]);
      m_collection->addPointCloud(m_pendingFilePath, chosen);
      m_showFieldSelector = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
      m_showFieldSelector = false;

    ImGui::EndPopup();
  }

private:
  CollectionScriptable *m_collection{nullptr};
  LabelingToolScriptable *m_labeling{nullptr};

  std::string m_pendingFilePath;
  std::vector<std::string> m_availableFields;
  std::vector<bool> m_selectedFields;
  bool m_showFieldSelector{false};
};