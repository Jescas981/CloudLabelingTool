#pragma once

#include "SceneController.h"
#include "SelectionTool.h"
#include <string>
#include <functional>

namespace PointCloudTool {

enum class EditorMode {
    Navigation,  // Normal camera navigation
    Selection    // Point selection mode
};

/**
 * @brief Manages all ImGui UI elements
 *
 * Responsibilities:
 * - Render menu bar
 * - Render dialogs (screenshot, recording, about)
 * - Render info panel
 * - Handle UI interactions
 */
class UIManager {
public:
    UIManager(SceneController* sceneController);
    ~UIManager() = default;

    // Main render method
    void render(int windowWidth, int windowHeight);

    // UI state
    bool wantsExit() const { return wantsExit_; }
    void setExitCallback(std::function<void()> callback) { exitCallback_ = callback; }

    // Editor mode
    EditorMode getEditorMode() const { return editorMode_; }
    void setEditorMode(EditorMode mode);

    // Selection tool access
    SelectionTool* getSelectionTool() { return &selectionTool_; }
    const SelectionTool* getSelectionTool() const { return &selectionTool_; }

    // Window dimensions (stored for selection tool)
    void setWindowDimensions(int width, int height) { windowWidth_ = width; windowHeight_ = height; }

private:
    // Menu bar
    void renderMenuBar();
    void renderFileMenu();
    void renderEditMenu();
    void renderSettingsMenu();
    void renderRenderingMenu();
    void renderLabelingMenu();
    void renderHelpMenu();

    // Dialogs
    void renderScreenshotDialog(int windowWidth, int windowHeight);
    void renderRecordingDialog(int windowWidth, int windowHeight);
    void renderAboutDialog();
    void renderInfoPanel();

    // Panels
    void renderObjectListPanel();
    void renderToolboxPanel();
    void renderLabelingToolbox();

    // Overlays
    void renderSelectionOverlay();
    void renderViewGizmo();

    SceneController* sceneController_;

    // UI state flags
    bool showScreenshotDialog_ = false;
    bool showRecordingDialog_ = false;
    bool showAboutDialog_ = false;
    bool showInfoPanel_ = true;
    bool showObjectList_ = true;
    bool showToolbox_ = true;
    bool showLabelingToolbox_ = false;
    bool showViewGizmo_ = true;
    bool enableAnnotationTools_ = false;
    bool verboseLogging_ = false;
    bool wantsExit_ = false;

    // Editor mode and tools
    EditorMode editorMode_ = EditorMode::Navigation;
    SelectionTool selectionTool_;
    bool additiveSelection_ = false;  // Ctrl key for additive selection

    // Labeling mode
    bool labelingModeEnabled_ = false;
    int activeLabelId_ = 0;
    bool overwriteLabelsMode_ = true;  // If true, overwrite existing labels; if false, only label unselected points

    // Window dimensions
    int windowWidth_ = 0;
    int windowHeight_ = 0;

    // Screenshot state
    char screenshotFilename_[256] = "";
    int screenshotFormat_ = 0; // 0=PNG, 1=JPG, 2=BMP

    // Recording state
    char recordingPath_[256] = "";
    int recordingFPS_ = 30;
    bool compileToVideo_ = false;

    // Currently selected object
    int selectedObjectIndex_ = 0;

    // Callbacks
    std::function<void()> exitCallback_;
};

} // namespace PointCloudTool
