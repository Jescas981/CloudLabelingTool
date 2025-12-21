#pragma once

#include "KeyCodes.h"
#include <utility>

namespace Perceptral {

// Static Input interface - platform independent
class Input {
public:
    // Keyboard
    static bool isKeyPressed(KeyCode key);

    // Mouse
    static bool isMouseButtonPressed(MouseButton button);
    static std::pair<float, float> getMousePosition();
    static float getMouseX();
    static float getMouseY();

    // Internal platform implementation
    static void init(void* window);
    static void shutdown();

private:
    static void* s_WindowHandle;
};

} // namespace Perceptral
