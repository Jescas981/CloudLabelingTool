#pragma once

#include <Perceptral/core/Event.h>
#include <functional>
#include <queue>
#include <memory>

struct GLFWwindow;

namespace Perceptral {

class EventManager {
public:
    EventManager();
    ~EventManager();

    void initialize(GLFWwindow* window);
    void processEvents();

    // Set event callback to receive all events
    using EventCallback = std::function<void(Event&)>;
    void setEventCallback(EventCallback callback) { eventCallback_ = callback; }

private:
    static void mouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallbackStatic(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowCloseCallbackStatic(GLFWwindow* window);
    static void windowSizeCallbackStatic(GLFWwindow* window, int width, int height);

    GLFWwindow* window_;
    EventCallback eventCallback_;

    // Event queue for processing
    std::queue<std::unique_ptr<Event>> eventQueue_;
};

} // namespace Perceptral
