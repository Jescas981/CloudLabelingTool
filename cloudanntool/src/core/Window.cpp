#include "core/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Enable MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return false;
    }

    glfwMakeContextCurrent(window_);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(window_, this);

    // Note: Framebuffer size callback is now handled by EventManager via glfwSetWindowSizeCallback
    // glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);

    // Enable vsync
    glfwSwapInterval(1);

    std::cout << "Window created: " << width_ << "x" << height_ << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    return true;
}

void Window::destroy()
{
    if (window_) {
        glfwDestroyWindow(window_);
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
