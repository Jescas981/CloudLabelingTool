#include "UIManager.h"
#include <Perceptral/core/Log.h>
#include <Perceptral/io/ImageExporter.h>
#include <imgui.h>
#include <nfd.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace PointCloudTool {

UIManager::UIManager(SceneController* sceneController)
    : sceneController_(sceneController)
{
    PC_CORE_INFO("UIManager initialized");
}

void UIManager::render(int windowWidth, int windowHeight)
{
    windowWidth_ = windowWidth;
    windowHeight_ = windowHeight;

    renderMenuBar();

    if (showInfoPanel_) {
        renderInfoPanel();
    }

    if (showObjectList_) {
        renderObjectListPanel();
    }

    if (showToolbox_) {
        renderToolboxPanel();
    }

    if (showLabelingToolbox_ && enableAnnotationTools_) {
        renderLabelingToolbox();
    }

    if (showScreenshotDialog_) {
        renderScreenshotDialog(windowWidth, windowHeight);
    }

    if (showRecordingDialog_) {
        renderRecordingDialog(windowWidth, windowHeight);
    }

    if (showAboutDialog_) {
        renderAboutDialog();
    }

    // Render selection rectangle overlay if in selection mode
    if (editorMode_ == EditorMode::Selection && selectionTool_.isSelecting()) {
        renderSelectionOverlay();
    }

    // Render view gizmo
    if (showViewGizmo_) {
        renderViewGizmo();
    }
}

void UIManager::setEditorMode(EditorMode mode)
{
    editorMode_ = mode;
    if (mode == EditorMode::Navigation) {
        selectionTool_.cancelSelection();
    }
    PC_CORE_INFO("Editor mode changed to: {}",
                 mode == EditorMode::Navigation ? "Navigation" : "Selection");
}

void UIManager::renderMenuBar()
{
    if (ImGui::BeginMainMenuBar()) {
        renderFileMenu();
        renderEditMenu();
        renderLabelingMenu();
        renderSettingsMenu();
        renderRenderingMenu();
        renderHelpMenu();

        ImGui::EndMainMenuBar();
    }
}

void UIManager::renderFileMenu()
{
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Load Point Cloud...", "Ctrl+O")) {
            nfdchar_t* outPath = nullptr;
            nfdfilteritem_t filterItem[1] = { { "Point Cloud Files", "pcd,ply" } };
            nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, nullptr);

            if (result == NFD_OKAY) {
                PC_CORE_INFO("Selected file: {}", outPath);
                if (sceneController_->loadPointCloudFromFile(outPath)) {
                    PC_CORE_INFO("Successfully loaded point cloud from {}", outPath);
                } else {
                    PC_CORE_ERROR("Failed to load point cloud from {}", outPath);
                }
                NFD_FreePath(outPath);
            } else if (result == NFD_CANCEL) {
                PC_CORE_INFO("User cancelled file dialog");
            } else {
                PC_CORE_ERROR("File dialog error: {}", NFD_GetError());
            }
        }

        ImGui::Separator();

        // Save Point Cloud with Labels
        if (ImGui::BeginMenu("Save Point Cloud As...")) {
            auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);
            if (selectedObj) {
                if (ImGui::MenuItem("PLY (Binary with Labels)")) {
                    nfdchar_t* outPath = nullptr;
                    nfdfilteritem_t filterItem[1] = { { "PLY Files", "ply" } };
                    nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, "cloud_labeled.ply");

                    if (result == NFD_OKAY) {
                        PC_CORE_INFO("Saving to: {}", outPath);
                        if (sceneController_->savePointCloudToFile(selectedObj->getName(), outPath, "ply")) {
                            PC_CORE_INFO("Successfully saved PLY file with labels");
                        } else {
                            PC_CORE_ERROR("Failed to save PLY file");
                        }
                        NFD_FreePath(outPath);
                    }
                }

                if (ImGui::MenuItem("PLY (ASCII with Labels)")) {
                    nfdchar_t* outPath = nullptr;
                    nfdfilteritem_t filterItem[1] = { { "PLY Files", "ply" } };
                    nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, "cloud_labeled.ply");

                    if (result == NFD_OKAY) {
                        PC_CORE_INFO("Saving to: {}", outPath);
                        if (sceneController_->savePointCloudToFile(selectedObj->getName(), outPath, "ply_ascii")) {
                            PC_CORE_INFO("Successfully saved PLY ASCII file with labels");
                        } else {
                            PC_CORE_ERROR("Failed to save PLY ASCII file");
                        }
                        NFD_FreePath(outPath);
                    }
                }

                if (ImGui::MenuItem("XYZL (Text Format)")) {
                    nfdchar_t* outPath = nullptr;
                    nfdfilteritem_t filterItem[1] = { { "XYZL Files", "xyzl,txt" } };
                    nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, "cloud_labeled.xyzl");

                    if (result == NFD_OKAY) {
                        PC_CORE_INFO("Saving to: {}", outPath);
                        if (sceneController_->savePointCloudToFile(selectedObj->getName(), outPath, "xyzl")) {
                            PC_CORE_INFO("Successfully saved XYZL file");
                        } else {
                            PC_CORE_ERROR("Failed to save XYZL file");
                        }
                        NFD_FreePath(outPath);
                    }
                }
            } else {
                ImGui::Text("No point cloud loaded");
            }
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Exit", "ESC")) {
            wantsExit_ = true;
            if (exitCallback_) {
                exitCallback_();
            }
        }

        ImGui::EndMenu();
    }
}

void UIManager::renderSettingsMenu()
{
    if (ImGui::BeginMenu("Settings")) {
        ImGui::Text("Camera");
        ImGui::Separator();

        auto camera = sceneController_->getCamera();
        float fov = camera->getFOV();
        if (ImGui::SliderFloat("FOV", &fov, 10.0f, 120.0f, "%.1f°")) {
            camera->setPerspective(fov, camera->getAspectRatio(),
                                 camera->getNear(), camera->getFar());
            PC_CORE_TRACE("FOV changed to: {:.1f}°", fov);
        }

        if (ImGui::MenuItem("Reset Camera", "R")) {
            sceneController_->resetCamera();
            PC_CORE_INFO("Camera reset");
        }

        ImGui::Spacing();
        ImGui::Text("Rendering");
        ImGui::Separator();

        // Apply point size to selected object
        auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);
        if (selectedObj) {
            auto* comp = selectedObj->getComponent();
            if (comp) {
                ImGui::SliderFloat("Point Size", &comp->pointSize, 1.0f, 20.0f, "%.1f");
            }
        }

        ImGui::Spacing();
        ImGui::Text("Tools");
        ImGui::Separator();

        if (ImGui::MenuItem("Enable Annotation Tools", nullptr, enableAnnotationTools_)) {
            enableAnnotationTools_ = !enableAnnotationTools_;
            if (enableAnnotationTools_) {
                showLabelingToolbox_ = true;
                PC_CORE_INFO("Annotation tools enabled");
            } else {
                showLabelingToolbox_ = false;
                PC_CORE_INFO("Annotation tools disabled");
            }
        }

        ImGui::Spacing();
        ImGui::Text("Logging");
        ImGui::Separator();

        if (ImGui::MenuItem("Toggle Verbose Logging", "V", verboseLogging_)) {
            verboseLogging_ = !verboseLogging_;
            if (verboseLogging_) {
                Perceptral::Log::setDebug();
                PC_CORE_INFO("Verbose logging enabled");
            } else {
                Perceptral::Log::setInfo();
                PC_CORE_INFO("Verbose logging disabled");
            }
        }

        ImGui::EndMenu();
    }
}

void UIManager::renderRenderingMenu()
{
    if (ImGui::BeginMenu("Rendering")) {
        ImGui::Text("Capture");
        ImGui::Separator();

        if (ImGui::MenuItem("Screenshot...", "F11")) {
            showScreenshotDialog_ = true;
        }

        ImGui::Spacing();

        bool isRecording = Perceptral::ImageExporter::isRecording();
        if (ImGui::MenuItem(isRecording ? "Stop Recording" : "Start Recording...", "F12")) {
            if (isRecording) {
                Perceptral::ImageExporter::stopRecording(compileToVideo_);
                PC_CORE_INFO("Recording stopped. {} frames captured",
                           Perceptral::ImageExporter::getFrameCount());
            } else {
                showRecordingDialog_ = true;
            }
        }

        if (isRecording) {
            ImGui::Text("Recording: %d frames", Perceptral::ImageExporter::getFrameCount());
        }

        ImGui::Spacing();
        ImGui::Text("Color Mode");
        ImGui::Separator();

        // Apply color mode to selected object
        auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);
        if (selectedObj) {
            auto* comp = selectedObj->getComponent();
            if (comp) {
                const char* colorModeNames[] = { "RGB", "Flat Color", "Axis X", "Axis Y", "Axis Z", "Gradient" };
                int currentColorMode = static_cast<int>(comp->colorMode);

                if (ImGui::Combo("Mode", &currentColorMode, colorModeNames, IM_ARRAYSIZE(colorModeNames))) {
                    comp->colorMode = static_cast<Perceptral::PointCloudColorMode>(currentColorMode);
                    PC_CORE_INFO("Color mode changed to: {}", colorModeNames[currentColorMode]);
                }

                // Flat color picker
                if (comp->colorMode == Perceptral::PointCloudColorMode::FlatColor) {
                    float color[3] = { comp->flatColor.x(), comp->flatColor.y(), comp->flatColor.z() };
                    if (ImGui::ColorEdit3("Color", color)) {
                        comp->flatColor = Eigen::Vector3f(color[0], color[1], color[2]);
                    }
                }
            }
        }

        ImGui::Spacing();
        ImGui::Text("Selection");
        ImGui::Separator();

        if (selectedObj) {
            auto* comp = selectedObj->getComponent();
            if (comp) {
                // Show selection color picker
                float selColor[3] = { comp->selectionColor.x(), comp->selectionColor.y(), comp->selectionColor.z() };
                if (ImGui::ColorEdit3("Selection Color", selColor)) {
                    comp->selectionColor = Eigen::Vector3f(selColor[0], selColor[1], selColor[2]);
                }

                // Show number of selected points
                if (!comp->selectionMask.empty()) {
                    size_t selectedCount = 0;
                    for (auto val : comp->selectionMask) {
                        if (val != 0) selectedCount++;
                    }
                    ImGui::Text("Selected Points: %zu / %zu", selectedCount, comp->selectionMask.size());
                }
            }
        }

        ImGui::EndMenu();
    }
}

void UIManager::renderLabelingMenu()
{
    if (ImGui::BeginMenu("Labeling")) {
        auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);

        if (!selectedObj) {
            ImGui::TextDisabled("No object selected");
            ImGui::EndMenu();
            return;
        }

        auto* comp = selectedObj->getComponent();
        if (!comp) {
            ImGui::TextDisabled("No point cloud component");
            ImGui::EndMenu();
            return;
        }

        // Enable labeling mode toggle
        if (ImGui::MenuItem("Enable Labeling Mode", nullptr, labelingModeEnabled_)) {
            labelingModeEnabled_ = !labelingModeEnabled_;
            if (labelingModeEnabled_) {
                comp->showLabels = true;  // Automatically show labels when enabled
                enableAnnotationTools_ = true;
                showLabelingToolbox_ = true;
                PC_CORE_INFO("Labeling mode enabled - all points rendered by label (default: 0 Unclassified)");
            } else {
                comp->showLabels = false;
                PC_CORE_INFO("Labeling mode disabled");
            }
        }

        // Overwrite mode toggle
        if (ImGui::MenuItem("Overwrite Existing Labels", nullptr, overwriteLabelsMode_, labelingModeEnabled_)) {
            overwriteLabelsMode_ = !overwriteLabelsMode_;
            PC_CORE_INFO("Overwrite mode: {}", overwriteLabelsMode_ ? "ON (will overwrite all selected points)" : "OFF (will preserve existing labels)");
        }

        ImGui::Separator();

        // Active label selection (only when labeling mode is enabled)
        ImGui::BeginDisabled(!labelingModeEnabled_);

        auto labelDef = selectedObj->getLabelDefinition();
        if (labelDef && ImGui::BeginMenu("Active Label")) {
            const auto& labels = labelDef->getAllLabels();

            ImGui::Text("Select label to assign:");
            ImGui::Separator();

            for (const auto& label : labels) {
                bool isActive = (activeLabelId_ == label.id);

                // Color indicator
                ImVec4 color(label.color.x(), label.color.y(), label.color.z(), 1.0f);
                ImGui::PushStyleColor(ImGuiCol_Text, color);

                std::string menuLabel = std::to_string(label.id) + ": " + label.name;
                if (ImGui::MenuItem(menuLabel.c_str(), nullptr, isActive)) {
                    activeLabelId_ = label.id;
                    PC_CORE_INFO("Active label changed to: {} - {}", label.id, label.name);
                }

                ImGui::PopStyleColor();
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        // Quick assign to selected and clear (only if points are selected)
        bool hasSelection = selectedObj->hasSelection();
        ImGui::BeginDisabled(!hasSelection);

        if (ImGui::MenuItem("Apply & Clear Selection")) {
            selectedObj->assignLabelToSelected(activeLabelId_, overwriteLabelsMode_);
            selectedObj->clearSelection();
            PC_CORE_INFO("Applied label {} and cleared selection", activeLabelId_);
        }

        ImGui::EndDisabled();

        if (!hasSelection && labelingModeEnabled_) {
            ImGui::TextDisabled("(No points selected)");
        }

        ImGui::EndDisabled();

        ImGui::Separator();

        // Label presets
        if (ImGui::BeginMenu("Label Presets")) {
            if (labelDef) {
                if (ImGui::MenuItem("Default Labels")) {
                    labelDef->loadDefaultLabels();
                    PC_CORE_INFO("Loaded default labels");
                }
                if (ImGui::MenuItem("LiDAR Labels")) {
                    labelDef->loadLiDARLabels();
                    PC_CORE_INFO("Loaded LiDAR labels");
                }
                if (ImGui::MenuItem("Semantic Segmentation")) {
                    labelDef->loadSemanticSegmentationLabels();
                    PC_CORE_INFO("Loaded semantic segmentation labels");
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
}

void UIManager::renderHelpMenu()
{
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About")) {
            showAboutDialog_ = true;
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Show Info Panel", nullptr, showInfoPanel_)) {
            showInfoPanel_ = !showInfoPanel_;
        }

        if (ImGui::MenuItem("Show Object List", nullptr, showObjectList_)) {
            showObjectList_ = !showObjectList_;
        }

        if (ImGui::MenuItem("Show Toolbox", nullptr, showToolbox_)) {
            showToolbox_ = !showToolbox_;
        }

        ImGui::BeginDisabled(!enableAnnotationTools_);
        if (ImGui::MenuItem("Show Labeling Toolbox", nullptr, showLabelingToolbox_)) {
            showLabelingToolbox_ = !showLabelingToolbox_;
        }
        ImGui::EndDisabled();

        if (ImGui::MenuItem("Show View Gizmo", nullptr, showViewGizmo_)) {
            showViewGizmo_ = !showViewGizmo_;
        }

        ImGui::EndMenu();
    }
}

void UIManager::renderInfoPanel()
{
    ImGui::Begin("Info", &showInfoPanel_, ImGuiWindowFlags_AlwaysAutoResize);

    auto camera = sceneController_->getCamera();

    // Camera info
    ImGui::Text("Camera");
    ImGui::Separator();
    ImGui::Text("Distance: %.2f", camera->getOrbitDistance());
    auto pos = camera->getPosition();
    ImGui::Text("Position: [%.2f, %.2f, %.2f]", pos.x(), pos.y(), pos.z());
    ImGui::Text("FOV: %.1f°", camera->getFOV());

    ImGui::Spacing();

    // Scene info
    ImGui::Text("Scene");
    ImGui::Separator();
    ImGui::Text("Objects: %zu", sceneController_->getObjectCount());

    ImGui::End();
}

void UIManager::renderObjectListPanel()
{
    ImGui::Begin("Objects", &showObjectList_);

    const auto& objects = sceneController_->getObjects();

    if (objects.empty()) {
        ImGui::Text("No objects in scene");
    } else {
        ImGui::Text("Total objects: %zu", objects.size());
        ImGui::Separator();
        ImGui::Spacing();

        for (size_t i = 0; i < objects.size(); ++i) {
            auto* obj = objects[i].get();

            ImGui::PushID(static_cast<int>(i));

            // Visibility checkbox
            bool visible = obj->isVisible();
            if (ImGui::Checkbox("##visible", &visible)) {
                obj->setVisible(visible);
                PC_CORE_INFO("Object '{}' visibility: {}", obj->getName(), visible ? "visible" : "hidden");
            }

            ImGui::SameLine();

            // Object name as selectable
            bool isSelected = (static_cast<int>(i) == selectedObjectIndex_);
            if (ImGui::Selectable(obj->getName().c_str(), isSelected)) {
                selectedObjectIndex_ = static_cast<int>(i);
            }

            // Tooltip with object info
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Points: %zu", obj->getPointCount());
                auto min = obj->getMin();
                auto max = obj->getMax();
                ImGui::Text("Bounds:");
                ImGui::Text("  Min: [%.2f, %.2f, %.2f]", min.x(), min.y(), min.z());
                ImGui::Text("  Max: [%.2f, %.2f, %.2f]", max.x(), max.y(), max.z());
                ImGui::Text("Visible: %s", visible ? "Yes" : "No");
                ImGui::EndTooltip();
            }

            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Action buttons
        if (ImGui::Button("Frame Selected")) {
            if (selectedObjectIndex_ >= 0 && selectedObjectIndex_ < static_cast<int>(objects.size())) {
                sceneController_->frameCamera(objects[selectedObjectIndex_]->getName());
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Frame All")) {
            sceneController_->frameCameraAll();
        }

        ImGui::Spacing();

        // Bulk visibility controls
        if (ImGui::Button("Show All")) {
            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i]->setVisible(true);
            }
            PC_CORE_INFO("All objects set to visible");
        }

        ImGui::SameLine();

        if (ImGui::Button("Hide All")) {
            for (size_t i = 0; i < objects.size(); ++i) {
                objects[i]->setVisible(false);
            }
            PC_CORE_INFO("All objects set to hidden");
        }
    }

    ImGui::End();
}

void UIManager::renderScreenshotDialog(int windowWidth, int windowHeight)
{
    ImGui::Begin("Save Screenshot", &showScreenshotDialog_, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Save screenshot of the current view");
    ImGui::Spacing();

    ImGui::InputText("Filename", screenshotFilename_, sizeof(screenshotFilename_));

    ImGui::Spacing();
    ImGui::Text("Format:");
    ImGui::RadioButton("PNG", &screenshotFormat_, 0); ImGui::SameLine();
    ImGui::RadioButton("JPG", &screenshotFormat_, 1); ImGui::SameLine();
    ImGui::RadioButton("BMP", &screenshotFormat_, 2);

    ImGui::Spacing();

    if (ImGui::Button("Save", ImVec2(120, 0))) {
        std::string filename(screenshotFilename_);
        if (filename.empty()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << "screenshot_" << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
            filename = ss.str();
        }

        const char* extensions[] = { ".png", ".jpg", ".bmp" };
        filename += extensions[screenshotFormat_];

        if (Perceptral::ImageExporter::saveScreenshot(filename, windowWidth, windowHeight)) {
            PC_CORE_INFO("Screenshot saved: {}", filename);
            showScreenshotDialog_ = false;
        } else {
            PC_CORE_ERROR("Failed to save screenshot");
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        showScreenshotDialog_ = false;
    }

    ImGui::End();
}

void UIManager::renderRecordingDialog(int windowWidth, int windowHeight)
{
    ImGui::Begin("Start Recording", &showRecordingDialog_, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Record frames to create a video");
    ImGui::Spacing();

    ImGui::InputText("Output Directory", recordingPath_, sizeof(recordingPath_));
    ImGui::SliderInt("FPS", &recordingFPS_, 15, 60);
    ImGui::Checkbox("Compile to video with ffmpeg", &compileToVideo_);

    ImGui::Spacing();

    if (ImGui::Button("Start Recording", ImVec2(150, 0))) {
        std::string path(recordingPath_);
        if (path.empty()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << "recording_" << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
            path = ss.str();
        }

        Perceptral::ImageExporter::startRecording(path, windowWidth, windowHeight, recordingFPS_);
        PC_CORE_INFO("Recording started to: {}", path);
        showRecordingDialog_ = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
        showRecordingDialog_ = false;
    }

    ImGui::End();
}

void UIManager::renderAboutDialog()
{
    ImGui::Begin("About Point Cloud Tool", &showAboutDialog_, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Point Cloud Tool v1.0");
    ImGui::Text("Built on Perceptral Engine");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Camera Controls:");
    ImGui::BulletText("Left Mouse: Rotate");
    ImGui::BulletText("Middle Mouse / Shift+Left: Pan");
    ImGui::BulletText("Right Mouse / Ctrl+Left: Zoom");
    ImGui::BulletText("Mouse Wheel: Zoom");
    ImGui::BulletText("R: Reset camera");

    ImGui::Spacing();
    ImGui::Text("Keyboard Shortcuts:");
    ImGui::BulletText("F11: Screenshot");
    ImGui::BulletText("F12: Record");
    ImGui::BulletText("V: Verbose logging");
    ImGui::BulletText("ESC: Exit");

    ImGui::Spacing();

    if (ImGui::Button("Close")) {
        showAboutDialog_ = false;
    }

    ImGui::End();
}

void UIManager::renderEditMenu()
{
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Navigation Mode", "Q", editorMode_ == EditorMode::Navigation)) {
            setEditorMode(EditorMode::Navigation);
        }

        if (ImGui::MenuItem("Selection Mode", "W", editorMode_ == EditorMode::Selection)) {
            setEditorMode(EditorMode::Selection);
        }

        ImGui::Separator();

        auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);
        bool hasSelection = selectedObj && selectedObj->hasSelection();

        if (ImGui::MenuItem("Clear Selection", "Ctrl+D", false, hasSelection)) {
            if (selectedObj) {
                selectedObj->clearSelection();
            }
        }

        ImGui::EndMenu();
    }
}

void UIManager::renderToolboxPanel()
{
    ImGui::Begin("Toolbox", &showToolbox_, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Editor Mode");
    ImGui::Separator();
    ImGui::Spacing();

    // Mode selection
    if (ImGui::RadioButton("Navigation (Q)", editorMode_ == EditorMode::Navigation)) {
        setEditorMode(EditorMode::Navigation);
    }

    if (ImGui::RadioButton("Selection (W)", editorMode_ == EditorMode::Selection)) {
        setEditorMode(EditorMode::Selection);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Selection tools (only enabled in selection mode)
    ImGui::BeginDisabled(editorMode_ != EditorMode::Selection);

    ImGui::Text("Selection Tools");
    ImGui::Separator();
    ImGui::Spacing();

    // Selection mode toggle
    auto currentMode = selectionTool_.getSelectionMode();
    if (ImGui::RadioButton("Rectangle Selection", currentMode == SelectionMode::Rectangle)) {
        selectionTool_.setSelectionMode(SelectionMode::Rectangle);
    }
    if (ImGui::RadioButton("Lasso Selection", currentMode == SelectionMode::Lasso)) {
        selectionTool_.setSelectionMode(SelectionMode::Lasso);
    }

    ImGui::Spacing();
    ImGui::Text("Controls:");
    ImGui::BulletText("Click and drag to select");
    // ImGui::BulletText("Hold Shift to add points");
    ImGui::BulletText("Hold Ctrl to remove points");
    ImGui::BulletText("Press clear to reset selection");

    ImGui::Spacing();

    // Selection info
    auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);
    if (selectedObj) {
        ImGui::Text("Object: %s", selectedObj->getName().c_str());

        if (selectedObj->hasSelection()) {
            ImGui::Text("Selected points: %zu / %zu",
                       selectedObj->getSelectionCount(),
                       selectedObj->getPointCount());

            if (ImGui::Button("Clear Selection")) {
                selectedObj->clearSelection();
            }
        } else {
            ImGui::TextDisabled("No points selected");
        }
    } else {
        ImGui::TextDisabled("No object selected");
    }

    ImGui::EndDisabled();

    ImGui::End();
}

void UIManager::renderLabelingToolbox()
{
    ImGui::Begin("Labeling Toolbox", &showLabelingToolbox_, ImGuiWindowFlags_AlwaysAutoResize);

    auto* selectedObj = sceneController_->getObjectAt(selectedObjectIndex_);

    if (!selectedObj) {
        ImGui::TextDisabled("No object selected");
        ImGui::Text("Select a point cloud to begin labeling");
        ImGui::End();
        return;
    }

    auto* comp = selectedObj->getComponent();
    if (!comp) {
        ImGui::TextDisabled("No point cloud component");
        ImGui::End();
        return;
    }

    // Label display toggle
    ImGui::Text("Display");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Checkbox("Show Labels", &comp->showLabels)) {
        PC_CORE_INFO("Label display: {}", comp->showLabels ? "ON" : "OFF");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Overwrite mode toggle
    ImGui::Text("Options");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Checkbox("Overwrite Existing Labels", &overwriteLabelsMode_)) {
        PC_CORE_INFO("Overwrite mode: {}", overwriteLabelsMode_ ? "ON (will overwrite all selected points)" : "OFF (will preserve existing labels)");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Label assignment section (only if points are selected)
    ImGui::Text("Label Assignment");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginDisabled(!selectedObj->hasSelection());

    if (selectedObj->hasSelection()) {
        ImGui::Text("Selected points: %zu / %zu",
                   selectedObj->getSelectionCount(),
                   selectedObj->getPointCount());
    } else {
        ImGui::TextDisabled("No points selected");
        ImGui::Text("Use Selection Mode (W) to select points");
    }

    ImGui::Spacing();

    auto labelDef = selectedObj->getLabelDefinition();
    if (labelDef) {
        const auto& labels = labelDef->getAllLabels();

        // Display quick assign buttons for first 6 labels
        ImGui::Text("Quick Assign:");
        int buttonsPerRow = 2;
        int buttonCount = 0;
        for (const auto& label : labels) {
            if (buttonCount >= 6) break; // Only show first 6 for quick access

            // Color button for label
            ImVec4 btnColor(label.color.x(), label.color.y(), label.color.z(), 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btnColor.x * 1.2f, btnColor.y * 1.2f, btnColor.z * 1.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(btnColor.x * 0.8f, btnColor.y * 0.8f, btnColor.z * 0.8f, 1.0f));

            std::string buttonLabel = std::to_string(label.id) + ": " + label.name;
            if (ImGui::Button(buttonLabel.c_str(), ImVec2(150, 0))) {
                size_t numSelected = selectedObj->getSelectionCount();
                selectedObj->assignLabelToSelected(label.id, overwriteLabelsMode_);
                selectedObj->clearSelection();
                PC_CORE_INFO("Assigned label {} to {} points and cleared selection", label.id, numSelected);
            }

            ImGui::PopStyleColor(3);

            buttonCount++;
            if (buttonCount % buttonsPerRow != 0) {
                ImGui::SameLine();
            }
        }
    }

    ImGui::EndDisabled();

    // Visibility controls section
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Visibility Controls");
    ImGui::Separator();
    ImGui::Spacing();

    if (labelDef) {
        const auto& labels = labelDef->getAllLabels();

        // Show All button
        if (ImGui::Button("Show All labels", ImVec2(150, 0))) {
            selectedObj->showAllLabels();
            PC_CORE_INFO("All labels shown");
        }

        if (ImGui::Button("Focus select", ImVec2(150, 0))) {
            selectedObj->showSelection();
            PC_CORE_INFO("Focus on selection only");
        }

        ImGui::Spacing();

        // Hide/Show buttons for each label
        ImGui::Text("Toggle visibility:");
        int visButtonsPerRow = 2;
        int visButtonCount = 0;
        for (const auto& label : labels) {
            if (visButtonCount >= 6) break; // Only show first 6 for quick access

            bool isHidden = selectedObj->isLabelHidden(label.id);

            // Color indicator
            ImVec4 btnColor(label.color.x(), label.color.y(), label.color.z(), 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btnColor.x * 1.2f, btnColor.y * 1.2f, btnColor.z * 1.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(btnColor.x * 0.8f, btnColor.y * 0.8f, btnColor.z * 0.8f, 1.0f));

            std::string buttonLabel = isHidden ? "Show " : "Hide ";
            buttonLabel += std::to_string(label.id) + ": " + label.name;

            if (ImGui::Button(buttonLabel.c_str(), ImVec2(150, 0))) {
                if (isHidden) {
                    selectedObj->showLabel(label.id);
                } else {
                    selectedObj->hideLabel(label.id);
                }
            }

            ImGui::PopStyleColor(3);

            visButtonCount++;
            if (visButtonCount % visButtonsPerRow != 0) {
                ImGui::SameLine();
            }
        }
    }

    ImGui::End();
}

void UIManager::renderSelectionOverlay()
{
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    if (selectionTool_.getSelectionMode() == SelectionMode::Rectangle) {
        // Rectangle selection
        float x1, y1, x2, y2;
        selectionTool_.getSelectionRect(x1, y1, x2, y2);

        // Draw filled rectangle with transparency
        drawList->AddRectFilled(
            ImVec2(x1, y1),
            ImVec2(x2, y2),
            IM_COL32(100, 150, 255, 50)  // Blue with transparency
        );

        // Draw border
        drawList->AddRect(
            ImVec2(x1, y1),
            ImVec2(x2, y2),
            IM_COL32(100, 150, 255, 255),  // Blue border
            0.0f,
            0,
            2.0f  // Border thickness
        );
    } else {
        // Lasso selection
        const auto& lassoPath = selectionTool_.getLassoPath();
        if (lassoPath.size() >= 2) {
            // Convert to ImVec2 array for drawing
            std::vector<ImVec2> imguiPath;
            for (const auto& point : lassoPath) {
                imguiPath.push_back(ImVec2(point.x(), point.y()));
            }

            // If still drawing and have enough points, close the path temporarily to show filled area
            if (selectionTool_.isSelecting() && lassoPath.size() >= 3) {
                // Add temporary closing line to first point
                imguiPath.push_back(ImVec2(lassoPath[0].x(), lassoPath[0].y()));
            }

            // Draw filled polygon with transparency (if we have enough points)
            if (imguiPath.size() >= 3) {
                drawList->AddConvexPolyFilled(
                    imguiPath.data(),
                    static_cast<int>(imguiPath.size()),
                    IM_COL32(100, 150, 255, 50)  // Blue with transparency
                );
            }

            // Draw lasso path as connected lines (outline)
            for (size_t i = 1; i < imguiPath.size(); ++i) {
                drawList->AddLine(
                    imguiPath[i-1],
                    imguiPath[i],
                    IM_COL32(100, 150, 255, 255),  // Blue line
                    2.0f  // Line thickness
                );
            }
        }
    }
}

void UIManager::renderViewGizmo()
{
    // Position in bottom-right corner
    const float gizmoSize = 120.0f;
    const float margin = 20.0f;
    const float buttonSize = 30.0f;

    ImVec2 gizmoPos(windowWidth_ - gizmoSize - margin, windowHeight_ - gizmoSize - margin);

    ImGui::SetNextWindowPos(gizmoPos);
    ImGui::SetNextWindowSize(ImVec2(gizmoSize, gizmoSize));

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBackground;

    ImGui::Begin("ViewGizmo", nullptr, windowFlags);

    auto camera = sceneController_->getCamera();

    // Create a 3x3 grid of buttons
    // Layout:
    //   [  ] [Top] [  ]
    //   [L  ] [F] [R  ]
    //   [  ] [Bot] [  ]

    const float btnW = buttonSize;
    const float btnH = buttonSize;
    const float spacing = 2.0f;

    // Row 1
    ImGui::Dummy(ImVec2(btnW, btnH));
    ImGui::SameLine(0, spacing);

    if (ImGui::Button("T", ImVec2(btnW, btnH))) {
        camera->setViewTop();
        PC_CORE_INFO("View set to Top");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Top View");
    }

    // Row 2
    if (ImGui::Button("L", ImVec2(btnW, btnH))) {
        camera->setViewLeft();
        PC_CORE_INFO("View set to Left");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Left View");
    }

    ImGui::SameLine(0, spacing);

    if (ImGui::Button("F", ImVec2(btnW, btnH))) {
        camera->setViewFront();
        PC_CORE_INFO("View set to Front");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Front View");
    }

    ImGui::SameLine(0, spacing);

    if (ImGui::Button("R", ImVec2(btnW, btnH))) {
        camera->setViewRight();
        PC_CORE_INFO("View set to Right");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Right View");
    }

    // Row 3
    ImGui::Dummy(ImVec2(btnW, btnH));
    ImGui::SameLine(0, spacing);

    if (ImGui::Button("B", ImVec2(btnW, btnH))) {
        camera->setViewBottom();
        PC_CORE_INFO("View set to Bottom");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Bottom View");
    }

    ImGui::SameLine(0, spacing);

    if (ImGui::Button("Bk", ImVec2(btnW, btnH))) {
        camera->setViewBack();
        PC_CORE_INFO("View set to Back");
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Back View");
    }

    ImGui::End();
}

} // namespace PointCloudTool
