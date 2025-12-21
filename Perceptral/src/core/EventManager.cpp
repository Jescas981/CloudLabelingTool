#include <Perceptral/core/EventManager.h>
#include <Perceptral/core/KeyCodes.h>
#include <GLFW/glfw3.h>
#include <Perceptral/core/Macros.h>

namespace Perceptral {

// Helper function to convert GLFW key codes to our KeyCode enum
static KeyCode convertGLFWKey(int glfwKey) {
    return static_cast<KeyCode>(glfwKey);
}

// Helper function to convert GLFW mouse button to our MouseButton enum
static MouseButton convertGLFWMouseButton(int glfwButton) {
    return static_cast<MouseButton>(glfwButton);
}

EventManager::EventManager()
    : window_(nullptr)
{
}

EventManager::~EventManager()
{
}

void EventManager::initialize(GLFWwindow* window)
{
    window_ = window;

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window_, this);

    // Set callbacks
    glfwSetMouseButtonCallback(window_, mouseButtonCallbackStatic);
    glfwSetCursorPosCallback(window_, cursorPosCallbackStatic);
    glfwSetScrollCallback(window_, scrollCallbackStatic);
    glfwSetKeyCallback(window_, keyCallbackStatic);
    glfwSetWindowCloseCallback(window_, windowCloseCallbackStatic);
    glfwSetWindowSizeCallback(window_, windowSizeCallbackStatic);
}

void EventManager::processEvents()
{
    // Process all queued events
    while (!eventQueue_.empty()) {
        auto& event = eventQueue_.front();
        if (eventCallback_) {
            eventCallback_(*event);
        }
        eventQueue_.pop();
    }
}

void EventManager::mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods)
{
    UNUSED(mods);
    EventManager* manager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        MouseButton mouseButton = convertGLFWMouseButton(button);

        if (action == GLFW_PRESS) {
            manager->eventQueue_.push(std::make_unique<MouseButtonPressedEvent>(mouseButton));
        } else if (action == GLFW_RELEASE) {
            manager->eventQueue_.push(std::make_unique<MouseButtonReleasedEvent>(mouseButton));
        }
    }
}

void EventManager::cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos)
{
    EventManager* manager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->eventQueue_.push(std::make_unique<MouseMovedEvent>(static_cast<float>(xpos), static_cast<float>(ypos)));
    }
}

void EventManager::scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset)
{
    EventManager* manager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->eventQueue_.push(std::make_unique<MouseScrolledEvent>(static_cast<float>(xoffset), static_cast<float>(yoffset)));
    }
}

void EventManager::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    UNUSED(scancode);
    UNUSED(mods);

    EventManager* manager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        KeyCode keyCode = convertGLFWKey(key);

        if (action == GLFW_PRESS) {
            manager->eventQueue_.push(std::make_unique<KeyPressedEvent>(keyCode, 0));
        } else if (action == GLFW_RELEASE) {
            manager->eventQueue_.push(std::make_unique<KeyReleasedEvent>(keyCode));
        } else if (action == GLFW_REPEAT) {
            manager->eventQueue_.push(std::make_unique<KeyPressedEvent>(keyCode, 1));
        }
    }
}

void EventManager::windowCloseCallbackStatic(GLFWwindow* window)
{
    EventManager* manager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->eventQueue_.push(std::make_unique<WindowCloseEvent>());
    }
}

void EventManager::windowSizeCallbackStatic(GLFWwindow* window, int width, int height)
{
    EventManager* manager = static_cast<EventManager*>(glfwGetWindowUserPointer(window));
    if (manager) {
        manager->eventQueue_.push(std::make_unique<WindowResizeEvent>(width, height));
    }
}

} // namespace Perceptral
