#include <Perceptral/core/Application.h>
#include <Perceptral/core/DeltaTime.h>
#include <Perceptral/core/Event.h>
#include <Perceptral/core/KeyCodes.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/core/layers/RenderLayer.h>
#include <Perceptral/io/ImageExporter.h>
#include <Perceptral/scene/Scene.h>

#include "CameraController.h"
#include "SceneController.h"
#include "UIManager.h"

#include <memory>
#include <nfd.h>

using namespace Perceptral;
using namespace PointCloudTool;

/**
 * @brief Main scene for Point Cloud Tool application
 */
class PointCloudToolScene : public Scene {
public:
  PointCloudToolScene() : Scene("PointCloudToolScene") {}

  void onCreate() override {
    PC_INFO("Creating PointCloudToolScene...");
    PC_INFO("Scene created successfully!");
  }
};

/**
 * @brief Main application class for Point Cloud Tool
 *
 * This application demonstrates clean architecture with separated
 * responsibilities:
 * - SceneController: Manages objects in the scene
 * - CameraController: Handles camera input
 * - UIManager: Manages all UI elements
 * - PointCloudObject: Objects with their own logic and lifecycle
 */
class PointCloudToolApp : public Application {
public:
  PointCloudToolApp() : Application("Point Cloud Tool") {}

  void onInit() override {
    PC_INFO("=== Point Cloud Tool ===");
    PC_INFO("Initializing application...");

    // Initialize NFD (must be after window creation)
    if (NFD_Init() != NFD_OKAY) {
      PC_CORE_ERROR("Failed to initialize NFD: {}", NFD_GetError());
      stop();
      return;
    }
    PC_INFO("NFD initialized successfully");

    // Create scene
    auto scene = std::make_shared<PointCloudToolScene>();
    pushScene(scene);

    // Initialize controllers
    sceneController_ = std::make_unique<SceneController>(scene, getCamera());
    cameraController_ = std::make_unique<CameraController>(getCamera());
    uiManager_ = std::make_unique<UIManager>(sceneController_.get());

    cameraController_->setMode(CameraMode::Orbit);

    // Layers
    auto renderLayer =
        std::make_unique<RenderLayer>(*scene.get(), *getCamera());
    this->getLayerStack().pushLayer(std::move(renderLayer));

    // Set exit callback
    uiManager_->setExitCallback([this]() { this->stop(); });

    printControls();
    PC_INFO("Point Cloud Tool initialized successfully\n");
  }

  void onShutdown() override {
    PC_INFO("Shutting down Point Cloud Tool...");
    NFD_Quit();
    PC_INFO("NFD shutdown complete");
  }

  void onUpdate(DeltaTime deltaTime) override {
    // Update scene (updates all objects)
    // sceneController_->update(deltaTime);
    // cameraController_->onUpdate(deltaTime);

    // Capture frame if recording
    if (ImageExporter::isRecording()) {
      ImageExporter::captureFrame();
    }

    // FPS logging
    fpsCounter(deltaTime);
  }

  void onEvent(Event &e) override {
    Application::onEvent(e);

    // If ImGui handled the event, don't process further
    if (e.isHandled()) {
      return;
    }

    // Handle keyboard shortcuts
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<KeyPressedEvent>(
        [this](KeyPressedEvent &event) { return onKeyPressed(event); });

    // Handle selection tool events if in selection mode
    if (!e.isHandled() &&
        uiManager_->getEditorMode() == EditorMode::Selection) {
      dispatcher.dispatch<MouseButtonPressedEvent>(
          [this](MouseButtonPressedEvent &event) {
            return onSelectionMousePressed(event);
          });

      dispatcher.dispatch<MouseButtonReleasedEvent>(
          [this](MouseButtonReleasedEvent &event) {
            return onSelectionMouseReleased(event);
          });

      dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent &event) {
        return onSelectionMouseMoved(event);
      });
    }

    if (!e.isHandled() &&
        uiManager_->getEditorMode() == EditorMode::Navigation) {
      cameraController_->onEvent(e);
    }

    // Camera controls (if not handled by keyboard or selection tool)
  }

  void onImGuiRender() override {
    uiManager_->render(getWindow()->getWidth(), getWindow()->getHeight());
  }

private:
  bool onKeyPressed(KeyPressedEvent &event) {
    switch (event.getKeyCode()) {
    case KeyCode::Escape:
      PC_INFO("Exiting application...");
      stop();
      return true;

    case KeyCode::Q:
      uiManager_->setEditorMode(EditorMode::Navigation);
      return true;

    case KeyCode::W:
      uiManager_->setEditorMode(EditorMode::Selection);
      return true;

    case KeyCode::R:
      sceneController_->resetCamera();
      PC_INFO("Camera reset");
      return true;

    case KeyCode::V:
      verboseLogging_ = !verboseLogging_;
      if (verboseLogging_) {
        Log::setDebug();
        PC_INFO("Verbose logging enabled");
      } else {
        Log::setInfo();
        PC_INFO("Verbose logging disabled");
      }
      return true;

    case KeyCode::D:
      // Clear selection with Ctrl+D
      if (Input::isKeyPressed(KeyCode::LeftControl) ||
          Input::isKeyPressed(KeyCode::RightControl)) {
        // Clear selection on selected object
        auto *obj =
            sceneController_->getObjectAt(0); // TODO: Use actual selected index
        if (obj) {
          obj->clearSelection();
        }
        return true;
      }
      return false;

    case KeyCode::F11:
      // Screenshot handled by UIManager
      return false;

    case KeyCode::F12:
      // Recording handled by UIManager
      return false;

    default:
      return false;
    }
  }

  bool onSelectionMousePressed(MouseButtonPressedEvent &event) {
    if (event.getMouseButton() == MouseButton::Left) {
      auto mousePos = Input::getMousePosition();
      uiManager_->getSelectionTool()->startSelection(mousePos.first,
                                                     mousePos.second);
      return true;
    }
    return false;
  }

  bool onSelectionMouseReleased(MouseButtonReleasedEvent &event) {
    if (event.getMouseButton() == MouseButton::Left) {
      auto *selTool = uiManager_->getSelectionTool();
      if (selTool->isSelecting()) {
        selTool->endSelection();

        // Perform selection on the selected object
        auto *obj =
            sceneController_->getObjectAt(0); // TODO: Use actual selected index
        if (obj && obj->getPointCloud()) {
          bool shiftPressed = Input::isKeyPressed(KeyCode::LeftShift) ||
                              Input::isKeyPressed(KeyCode::RightShift);
          bool ctrlPressed = Input::isKeyPressed(KeyCode::LeftControl) ||
                             Input::isKeyPressed(KeyCode::RightControl);

          auto selectedIndices = selTool->selectPoints(
              obj->getPointCloud(), getCamera(), getWindow()->getWidth(),
              getWindow()->getHeight(),
              false // always pass false here, we handle additive/subtractive
                    // separately
          );

          if (ctrlPressed) {
            // Subtractive selection (remove points)
            obj->deselectPoints(selectedIndices);
          } else  {
            // Additive selection (add points)
            obj->selectPoints(selectedIndices, true);
          } 
        }

        return true;
      }
    }
    return false;
  }

  bool onSelectionMouseMoved(MouseMovedEvent &event) {
    auto *selTool = uiManager_->getSelectionTool();
    if (selTool->isSelecting()) {
      selTool->updateSelection(event.getX(), event.getY());
      return true;
    }
    return false;
  }

  void fpsCounter(DeltaTime deltaTime) {
    static double accumulatedTime = 0.0;
    static int frameCount = 0;
    accumulatedTime += deltaTime;
    frameCount++;

    if (accumulatedTime >= 5.0) {
      float avgFps = frameCount / accumulatedTime;

      size_t totalPoints = 0;
      for (size_t i = 0; i < sceneController_->getObjectCount(); ++i) {
        if (auto *obj = sceneController_->getObjectAt(i)) {
          totalPoints += obj->getPointCount();
        }
      }

      PC_INFO(
          "FPS: {:.1f} | Objects: {} | Points: {} | Camera Distance: {:.2f}",
          avgFps, sceneController_->getObjectCount(), totalPoints,
          getCamera()->getOrbitDistance());

      accumulatedTime = 0.0;
      frameCount = 0;
    }
  }

  void printControls() {
    PC_INFO("\nEditor Modes:");
    PC_INFO("  Q:                     Navigation mode (camera control)");
    PC_INFO("  W:                     Selection mode (point selection)");
    PC_INFO("\nCamera Controls (Navigation Mode):");
    PC_INFO("  Left Mouse Drag:       Rotate around target");
    PC_INFO("  Middle Mouse Drag:     Pan camera");
    PC_INFO("  Shift + Left Drag:     Pan camera");
    PC_INFO("  Right Mouse Drag:      Zoom in/out");
    PC_INFO("  Ctrl + Left Drag:      Zoom in/out");
    PC_INFO("  Mouse Wheel:           Zoom in/out");
    PC_INFO("  R:                     Reset camera view");
    PC_INFO("\nSelection Controls (Selection Mode):");
    PC_INFO("  Left Click + Drag:     Rectangle selection (replace)");
    PC_INFO("  Shift + Select:        Additive selection (add points)");
    PC_INFO("  Ctrl + Select:         Subtractive selection (remove points)");
    PC_INFO("  Ctrl + D:              Clear selection");
    PC_INFO("\nApplication:");
    PC_INFO("  F11:                   Screenshot");
    PC_INFO("  F12:                   Start/Stop recording");
    PC_INFO("  V:                     Toggle verbose logging");
    PC_INFO("  ESC:                   Exit");
  }

  // Controllers
  std::unique_ptr<SceneController> sceneController_;
  std::unique_ptr<CameraController> cameraController_;
  std::unique_ptr<UIManager> uiManager_;

  // State
  bool verboseLogging_ = false;
};

int main(int argc, char **argv) {
  PointCloudToolApp app;

  if (app.initialize(1280, 720)) {
    app.run();
  }

  return 0;
}
