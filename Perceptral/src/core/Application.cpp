#include <Perceptral/core/Application.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/core/Macros.h>
#include <Perceptral/core/Window.h>
#include <Perceptral/core/Input.h>
#include <Perceptral/renderer/Renderer.h>
#include <Perceptral/renderer/RenderAPI.h>
#include <Perceptral/core/Time.h>
#include <Perceptral/core/DeltaTime.h>

namespace Perceptral {

Application* Application::s_Instance = nullptr;

Application::Application(const std::string& name)
    : name_(name)
    , running_(false)
    , lastFrameTime_(0.0) {
    s_Instance = this;
}

Application::~Application() {
    shutdown();
}

bool Application::initialize(int width, int height) {
    // Initialize logger first
    Log::init();
    PC_CORE_INFO("Initializing Application: {}", name_);

    Time::initialize();
    PC_CORE_INFO("Time system initialized");

    window_ = std::make_unique<Window>();
    
    if (!window_->create(width, height, name_)) {
        PC_CORE_ERROR("Failed to create window");
        return false;
    }

    PC_CORE_INFO("Window created: {}x{}", width, height);

    // Initialize RenderAPI
    Renderer::setRenderAPI(RenderAPI::create());
    Renderer::init();
    Renderer::setClearColor(Eigen::Vector4f(0.1f, 0.1f, 0.1f, 1.0f));
    Renderer::setViewport(0, 0, width, height);
    PC_CORE_INFO("Render API initialized");

    // Initialize Input system (platform-independent)
    Input::init(window_->getHandle());
    PC_CORE_DEBUG("Input system initialized");

    // Initialize ImGui layer
    imguiLayer_ = std::make_unique<ImGuiLayer>();
    imguiLayer_->init(window_->getHandle());
    PC_CORE_DEBUG("ImGui layer initialized");

    // Create camera
    camera_ = std::make_unique<Camera>();
    camera_->setPosition(Eigen::Vector3f(0.0f, -5.0f, 0.0f));
    camera_->setTarget(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    camera_->setPerspective(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    PC_CORE_DEBUG("Camera initialized");

    // Create event manager and set callback
    eventManager_ = std::make_unique<EventManager>();
    eventManager_->initialize(window_->getHandle());
    eventManager_->setEventCallback([this](Event& e) {
        onEvent(e);
    });
    PC_CORE_DEBUG("Event manager initialized");

    // User initialization
    onInit();

    running_ = true;
    lastFrameTime_ = Time::getTime();

    PC_CORE_INFO("{} initialized successfully", name_);
    return true;
}

void Application::run() {
    while (running_ && !window_->shouldClose()) {
        // Poll events
        window_->pollEvents();
        eventManager_->processEvents();

        // Update
        float currentTime = Time::getTime();
        DeltaTime deltaTime = currentTime - lastFrameTime_;

        // Update all layers (bottom to top)
        for (Layer* layer : m_layerStack) {
            layer->onUpdate(deltaTime);
        }

        onUpdate(deltaTime);

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
    PC_CORE_INFO("Shutting down {}...", name_);

    // Destroy all scenes
    while (sceneManager_.hasScene()) {
        sceneManager_.popScene();
    }

    onShutdown();

     for (Layer* layer : m_layerStack) {
        layer->onDetach();
    }

    PC_CORE_DEBUG("Layers should go down");

    camera_.reset();
    eventManager_.reset();
    Input::shutdown();
    Renderer::shutdown();
    window_.reset();

    PC_CORE_INFO("Application shutdown complete");
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
    UNUSED(e);
    stop();
    return true;
}

bool Application::onWindowResize(WindowResizeEvent& e) {
    PC_CORE_DEBUG("Window resized: {}x{}", e.getWidth(), e.getHeight());
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

} // namespace Perceptral
