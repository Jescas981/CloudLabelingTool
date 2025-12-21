#include <Perceptral/ui/ImGuiLayer.h>
#include <Perceptral/core/Log.h>
#include <Perceptral/core/Event.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace Perceptral {

ImGuiLayer::ImGuiLayer()
    : initialized_(false)
    , showDemo_(false)
{
}

ImGuiLayer::~ImGuiLayer()
{
    if (initialized_) {
        shutdown();
    }
}

void ImGuiLayer::init(GLFWwindow* window, const char* glslVersion)
{
    if (initialized_) {
        PC_CORE_WARN("ImGuiLayer already initialized");
        return;
    }

    PC_CORE_INFO("Initializing ImGui layer...");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enable keyboard and gamepad controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Optional: Enable docking (requires ImGui docking branch)
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Alternative: ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    // Pass false to install_callbacks - we'll handle events manually through our event system
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glslVersion);

    initialized_ = true;
    PC_CORE_INFO("ImGui layer initialized successfully");
}

void ImGuiLayer::shutdown()
{
    if (!initialized_) {
        return;
    }

    PC_CORE_INFO("Shutting down ImGui layer...");

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    initialized_ = false;
    PC_CORE_DEBUG("ImGui layer shutdown complete");
}

void ImGuiLayer::begin()
{
    if (!initialized_) {
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Show demo window if enabled
    if (showDemo_) {
        ImGui::ShowDemoWindow(&showDemo_);
    }
}

void ImGuiLayer::end()
{
    if (!initialized_) {
        return;
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::onEvent(Event& e)
{
    if (!initialized_) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();

    // Forward events to ImGui manually since we're not using GLFW callbacks
    EventDispatcher dispatcher(e);

    // Mouse button events
    dispatcher.dispatch<MouseButtonPressedEvent>([&io](MouseButtonPressedEvent& event) {
        int button = static_cast<int>(event.getMouseButton());
        if (button >= 0 && button < 5) {
            io.AddMouseButtonEvent(button, true);
        }
        return false; // Don't mark as handled yet
    });

    dispatcher.dispatch<MouseButtonReleasedEvent>([&io](MouseButtonReleasedEvent& event) {
        int button = static_cast<int>(event.getMouseButton());
        if (button >= 0 && button < 5) {
            io.AddMouseButtonEvent(button, false);
        }
        return false;
    });

    // Mouse movement
    dispatcher.dispatch<MouseMovedEvent>([&io](MouseMovedEvent& event) {
        io.AddMousePosEvent(event.getX(), event.getY());
        return false;
    });

    // Mouse scroll
    dispatcher.dispatch<MouseScrolledEvent>([&io](MouseScrolledEvent& event) {
        io.AddMouseWheelEvent(event.getXOffset(), event.getYOffset());
        return false;
    });

    // Note: Key events omitted for now - ImGui UI works fine with just mouse input
    // Text input and keyboard navigation can be added later with proper key mapping

    // Mark event as handled if ImGui is capturing input
    if (e.isInCategory(EventCategoryMouse) && io.WantCaptureMouse) {
        e.setHandled();
    }

    if (e.isInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard) {
        e.setHandled();
    }
}

bool ImGuiLayer::wantsCaptureMouse() const
{
    if (!initialized_) {
        return false;
    }
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiLayer::wantsCaptureKeyboard() const
{
    if (!initialized_) {
        return false;
    }
    return ImGui::GetIO().WantCaptureKeyboard;
}

} // namespace Perceptral
