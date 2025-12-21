#include <Perceptral/core/Time.h>
#include <Perceptral/core/Macros.h>

#ifdef PC_WINDOW_API_GLFW
    #include <GLFW/glfw3.h>
#else
    #include <chrono>
#endif

namespace Perceptral {

double Time::s_startTime = 0.0;

void Time::initialize() {
#ifdef PC_WINDOW_API_GLFW
    s_startTime = glfwGetTime();
#else
    // Fallback to std::chrono for non-GLFW platforms
    auto now = std::chrono::high_resolution_clock::now();
    s_startTime = std::chrono::duration<double>(now.time_since_epoch()).count();
#endif
}

double Time::getTime() {
#ifdef PC_WINDOW_API_GLFW
    return static_cast<float>(glfwGetTime() - s_startTime);
#else
    // Fallback to std::chrono for non-GLFW platforms
    auto now = std::chrono::high_resolution_clock::now();
    double current = std::chrono::duration<double>(now.time_since_epoch()).count();
    return static_cast<double>(current - s_startTime);
#endif
}

double Time::getTimeMilliseconds() {
    return static_cast<double>(getTime()) * 1000.0;
}

} // namespace Perceptral