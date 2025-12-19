#include "core/Application.h"
#include "core/Log.h"
#include "core/Window.h"
#include "core/Input.h"
#include "renderer/Renderer.h"
#include "renderer/Renderer.h"
#include "scene/Systems.h"
#include <GLFW/glfw3.h>

namespace CloudCore {

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
    : name_(name)
    , running_(false)
    , deltaTime_(0.0)
    , lastFrameTime_(0.0)
    , fps_(0.0)
    , frameCount_(0)
    , fpsUpdateTime_(0.0) {
    s_Instance = this;
}

Application::~Application() {
    shutdown();
}

bool Application::initialize(int width, int height) {
    // Initialize logger first
    Log::init();
    CC_CORE_INFO("Initializing Application: {}", name_);

    // Initialize GLFW
    if (!glfwInit()) {
        CC_CORE_ERROR("Failed to initialize GLFW");
        return false;
    }
    CC_CORE_DEBUG("GLFW initialized");

    // Create window
    window_ = std::make_unique<Window>();
    if (!window_->create(width, height, name_)) {
        CC_CORE_ERROR("Failed to create window");
        glfwTerminate();
        return false;
    }
    CC_CORE_INFO("Window created: {}x{}", width, height);

    // Initialize RenderAPI
    Renderer::setRenderAPI(RenderAPI::create());
    Renderer::init();
    Renderer::setClearColor(Eigen::Vector4f(0.1f, 0.1f, 0.1f, 1.0f));
    Renderer::setViewport(0, 0, width, height);
    CC_CORE_INFO("Render API initialized");

    // Initialize Input system (platform-independent)
    Input::init(window_->getHandle());
    CC_CORE_DEBUG("Input system initialized");

    // Initialize ImGui layer
    imguiLayer_ = std::make_unique<ImGuiLayer>();
    imguiLayer_->init(window_->getHandle());
    CC_CORE_DEBUG("ImGui layer initialized");

    // Create camera
    camera_ = std::make_unique<Camera>();
    camera_->setPosition(Eigen::Vector3f(0.0f, -5.0f, 0.0f));
    camera_->setTarget(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    camera_->setPerspective(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    CC_CORE_DEBUG("Camera initialized");

    // Create event manager and set callback
    eventManager_ = std::make_unique<EventManager>();
    eventManager_->initialize(window_->getHandle());
    eventManager_->setEventCallback([this](Event& e) {
        onEvent(e);
    });
    CC_CORE_DEBUG("Event manager initialized");

    // User initialization
    onInit();

    running_ = true;
    lastFrameTime_ = glfwGetTime();
    fpsUpdateTime_ = lastFrameTime_;

    CC_CORE_INFO("{} initialized successfully", name_);
    return true;
}

void Application::run() {
    while (running_ && !window_->shouldClose()) {
        updateFrameTiming();

        // Poll events
        window_->pollEvents();
        eventManager_->processEvents();

        // Update
        Timestep timestep(static_cast<float>(deltaTime_));

        // Update all layers (bottom to top)
        for (Layer* layer : m_layerStack) {
            layer->onUpdate(timestep);
        }

        onUpdate(timestep);

        // Render all layers
        Renderer::clear();
        for (Layer* layer : m_layerStack) {
            layer->onRender();
        }
        onRender();

        // ImGui rendering
        imguiLayer_->begin();

        for (Layer* layer : m_layerStack) {
            layer->onImGuiRender();
        }

        onImGuiRender();
            
        imguiLayer_->end();
        window_->swapBuffers();
    }
}

void Application::shutdown() {
    CC_CORE_INFO("Shutting down {}...", name_);

    // Destroy all scenes
    while (sceneManager_.hasScene()) {
        sceneManager_.popScene();
    }

    onShutdown();

    // Clean up rendering systems before destroying OpenGL context
    PointCloudRenderSystem::shutdown();
    CC_CORE_DEBUG("Render systems shutdown");

    camera_.reset();
    eventManager_.reset();
    Input::shutdown();
    Renderer::shutdown();
    window_.reset();

    glfwTerminate();
    CC_CORE_INFO("Application shutdown complete");
    Log::shutdown();
}

void Application::onEvent(Event& e) {
    // Let ImGui handle events first
    imguiLayer_->onEvent(e);

    // If ImGui handled it, don't process further
    if (e.isHandled()) {
        return;
    }

    EventDispatcher dispatcher(e);

    // Dispatch to default handlers
    dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& event) {
        return onWindowClose(event);
    });

    dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& event) {
        return onWindowResize(event);
    });
}

bool Application::onWindowClose(WindowCloseEvent& e) {
    stop();
    return true;
}

bool Application::onWindowResize(WindowResizeEvent& e) {
    CC_CORE_DEBUG("Window resized: {}x{}", e.getWidth(), e.getHeight());
    Renderer::setViewport(0, 0, e.getWidth(), e.getHeight());
    window_->setHeight(e.getHeight());
    window_->setWidth(e.getWidth());
    if (camera_) {
        camera_->setPerspective(camera_->getFOV(),
                               static_cast<float>(e.getWidth()) / e.getHeight(),
                               camera_->getNear(),
                               camera_->getFar());
    }
    return false;
}

void Application::pushScene(std::shared_ptr<Scene> scene) {
    scene->setCamera(camera_.get());
    sceneManager_.pushScene(scene);
}

void Application::setScene(std::shared_ptr<Scene> scene) {
    scene->setCamera(camera_.get());
    sceneManager_.setScene(scene);
}

void Application::updateFrameTiming() {
    double currentTime = glfwGetTime();
    deltaTime_ = currentTime - lastFrameTime_;
    lastFrameTime_ = currentTime;

    // Update FPS counter
    frameCount_++;
    if (currentTime - fpsUpdateTime_ >= 1.0) {
        fps_ = frameCount_ / (currentTime - fpsUpdateTime_);
        frameCount_ = 0;
        fpsUpdateTime_ = currentTime;
    }
}

} // namespace CloudCore
