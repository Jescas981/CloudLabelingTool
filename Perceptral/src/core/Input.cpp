#include <Perceptral/core/Input.h>
#include <GLFW/glfw3.h>

namespace Perceptral {

void* Input::s_WindowHandle = nullptr;

void Input::init(void* window) {
    s_WindowHandle = window;
}

void Input::shutdown() {
    s_WindowHandle = nullptr;
}

// Convert KeyCode to GLFW key
static int keyCodeToGLFW(KeyCode key) {
    return static_cast<int>(key);
}

// Convert MouseButton to GLFW button
static int mouseButtonToGLFW(MouseButton button) {
    return static_cast<int>(button);
}

bool Input::isKeyPressed(KeyCode key) {
    if (!s_WindowHandle) return false;

    auto window = static_cast<GLFWwindow*>(s_WindowHandle);
    int state = glfwGetKey(window, keyCodeToGLFW(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(MouseButton button) {
    if (!s_WindowHandle) return false;

    auto window = static_cast<GLFWwindow*>(s_WindowHandle);
    int state = glfwGetMouseButton(window, mouseButtonToGLFW(button));
    return state == GLFW_PRESS;
}

std::pair<float, float> Input::getMousePosition() {
    if (!s_WindowHandle) return {0.0f, 0.0f};

    auto window = static_cast<GLFWwindow*>(s_WindowHandle);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return { static_cast<float>(xpos), static_cast<float>(ypos) };
}

float Input::getMouseX() {
    auto [x, y] = getMousePosition();
    return x;
}

float Input::getMouseY() {
    auto [x, y] = getMousePosition();
    return y;
}

} // namespace Perceptral
