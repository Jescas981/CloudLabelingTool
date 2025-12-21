#pragma once

#include <Perceptral/core/Event.h>
#include <memory>

struct GLFWwindow;

namespace Perceptral {

/**
 * @brief ImGui integration layer for CloudCore
 *
 * Handles ImGui initialization, event processing, and rendering.
 * This layer wraps ImGui's GLFW and OpenGL3 backends.
 */
class ImGuiLayer {
public:
    ImGuiLayer();
    ~ImGuiLayer();

    /**
     * @brief Initialize ImGui with GLFW and OpenGL3 backends
     * @param window GLFW window handle
     * @param glslVersion GLSL version string (e.g., "#version 330")
     */
    void init(GLFWwindow* window, const char* glslVersion = "#version 330");

    /**
     * @brief Shutdown ImGui and cleanup resources
     */
    void shutdown();

    /**
     * @brief Begin new ImGui frame
     * Call this at the beginning of each frame before ImGui calls
     */
    void begin();

    /**
     * @brief End ImGui frame and render
     * Call this at the end of each frame after all ImGui calls
     */
    void end();

    /**
     * @brief Process ImGui-related events
     * @param e Event to process
     */
    void onEvent(Event& e);

    /**
     * @brief Check if ImGui wants to capture mouse input
     * @return true if ImGui is using the mouse
     */
    bool wantsCaptureMouse() const;

    /**
     * @brief Check if ImGui wants to capture keyboard input
     * @return true if ImGui is using the keyboard
     */
    bool wantsCaptureKeyboard() const;

    /**
     * @brief Enable/disable ImGui demo window
     * @param show true to show demo window
     */
    void setShowDemo(bool show) { showDemo_ = show; }

    /**
     * @brief Check if demo window is shown
     * @return true if demo window is visible
     */
    bool isShowingDemo() const { return showDemo_; }

private:
    bool initialized_;
    bool showDemo_;
};

} // namespace Perceptral
