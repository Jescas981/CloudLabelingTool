#include "core/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "core/Log.h"

namespace CloudCore {

Window::Window()
    : window_(nullptr)
    , width_(0)
    , height_(0)
{
}

Window::~Window()
{
    destroy();
}

bool Window::create(int width, int height, const std::string& title)
{
    width_ = width;
    height_ = height;
    title_ = title;

    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Enable MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    if (!glfwInit()) {
        CC_CORE_ERROR("Failed to initialize GLFW");
        return false;
    }

    CC_CORE_DEBUG("GLFW initialized");

    // Create window
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!window_) {
        CC_CORE_ERROR("Failed to create window");
        return false;
    }

    glfwMakeContextCurrent(window_);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        CC_CORE_ERROR("Failed to initialize glad");
        return false;
    }

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window_, this);

    // Note: Framebuffer size callback is now handled by EventManager via glfwSetWindowSizeCallback
    // glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

    // Enable vsync
    glfwSwapInterval(1);

    CC_CORE_DEBUG("Window created: {} x {}" ,width_ ,height_);
    CC_CORE_DEBUG("Something: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

    return true;
}

void Window::destroy()
{
    if (window_) {
        glfwDestroyWindow(window_);
        glfwTerminate();
        window_ = nullptr;
    }
}

void Window::swapBuffers()
{
    glfwSwapBuffers(window_);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window_);
}

void Window::pollEvents()
{
    glfwPollEvents();
}

// void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height)
// {
//     Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));

//     if (win) {
//         win->width_ = width;
//         win->height_ = height;
//         if (win->resizeCallback_) {
//             win->resizeCallback_(width, height);
//         }
//     }
// }

} // namespace CloudCore
