#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace Perceptral {

class Window {
public:
    Window();
    ~Window();

    // Create and manage window
    bool create(int width, int height, const std::string& title);
    void destroy();

    // Window operations
    void swapBuffers();
    bool shouldClose() const;
    void pollEvents();

    // Getters
    GLFWwindow* getHandle() const { return window_; }
    inline int getWidth() const { return width_; }
    inline int getHeight() const { return height_; }
    inline void setWidth(int width){ width_=width; }
    inline void setHeight(int height){ height_=height; }
    float getAspectRatio() const { return static_cast<float>(width_) / height_; }

    // Callbacks
    using ResizeCallback = std::function<void(int, int)>;
    void setResizeCallback(ResizeCallback callback) { resizeCallback_ = callback; }

private:
    // static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* window_;
    int width_;
    int height_;
    std::string title_;

    ResizeCallback resizeCallback_;
};

} // namespace Perceptral
