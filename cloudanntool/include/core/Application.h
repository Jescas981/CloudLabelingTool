#pragma once

#include "Window.h"
#include "core/LayerStack.h"
#include "core/Macros.h"
#include "core/DeltaTime.h"
#include "Event.h"
#include "EventManager.h"
#include "Camera.h"
#include "scene/SceneManager.h"
#include "ui/ImGuiLayer.h"
#include <memory>
#include <string>

namespace CloudCore {

class Renderer;

// Main application class
class Application {
public:
    Application(const std::string& name = "CloudAnnotationApp");
    virtual ~Application();

    // Lifecycle
    bool initialize(int width = 1280, int height = 720);
    void run();
    void shutdown();

    // Event handling (override in derived class)
    virtual void onEvent(Event& e);

    // Scene management
    void pushScene(std::shared_ptr<Scene> scene);
    void setScene(std::shared_ptr<Scene> scene);
    SceneManager& getSceneManager() { return sceneManager_; }

    // Getters
    Window* getWindow() const { return window_.get(); }
    EventManager* getEventManager() const { return eventManager_.get(); }
    Camera* getCamera() const { return camera_.get(); }
    ImGuiLayer* getImGuiLayer() const { return imguiLayer_.get(); }
    LayerStack& getLayerStack(){return m_layerStack; }

    // Control
    void stop() { running_ = false; }
    bool isRunning() const { return running_; }

    // Singleton access
    static Application& get() { return *s_Instance; }

protected:
    virtual void onInit() {}
    virtual void onUpdate(DeltaTime deltaTime) {UNUSED(deltaTime);}
    virtual void onRender() {}
    virtual void onImGuiRender() {}
    virtual void onShutdown() {}

    // Default event handlers
    bool onWindowClose(WindowCloseEvent& e);
    bool onWindowResize(WindowResizeEvent& e);

private:
    static Application* s_Instance;
    
    LayerStack m_layerStack; 
    std::string name_;
    std::unique_ptr<Window> window_;
    std::unique_ptr<EventManager> eventManager_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<ImGuiLayer> imguiLayer_;

    SceneManager sceneManager_;

    bool running_;
    float lastFrameTime_;
};

} // namespace CloudCore
