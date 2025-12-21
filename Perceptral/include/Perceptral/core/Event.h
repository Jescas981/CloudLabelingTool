#pragma once

#include "KeyCodes.h"
#include <string>
#include <functional>

namespace Perceptral {

// Event types
enum class EventType {
    None = 0,
    // Window events
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    // Key events
    KeyPressed, KeyReleased, KeyTyped,
    // Mouse events
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

// Event categories (for filtering)
enum EventCategory {
    None = 0,
    EventCategoryApplication    = 1 << 0,
    EventCategoryInput          = 1 << 1,
    EventCategoryKeyboard       = 1 << 2,
    EventCategoryMouse          = 1 << 3,
    EventCategoryMouseButton    = 1 << 4
};

#define EVENT_CLASS_TYPE(type) \
    static EventType getStaticType() { return EventType::type; } \
    virtual EventType getEventType() const override { return getStaticType(); } \
    virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int getCategoryFlags() const override { return category; }

// Base Event class
class Event {
public:
    virtual ~Event() = default;

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const { return getName(); }

    bool isInCategory(EventCategory category) const {
        return getCategoryFlags() & category;
    }

    bool isHandled() const { return handled; }
    void setHandled() { handled = true; }

    bool handled = false;
};

// Window events
class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(unsigned int width, unsigned int height)
        : width_(width), height_(height) {}

    unsigned int getWidth() const { return width_; }
    unsigned int getHeight() const { return height_; }

    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)

private:
    unsigned int width_, height_;
};

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryApplication)
};

// Key events
class KeyEvent : public Event {
public:
    KeyCode getKeyCode() const { return keyCode_; }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    KeyEvent(KeyCode keycode) : keyCode_(keycode) {}
    KeyCode keyCode_;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(KeyCode keycode, int repeatCount)
        : KeyEvent(keycode), repeatCount_(repeatCount) {}

    int getRepeatCount() const { return repeatCount_; }

    EVENT_CLASS_TYPE(KeyPressed)

private:
    int repeatCount_;
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

    EVENT_CLASS_TYPE(KeyReleased)
};

// Mouse events
class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y) : mouseX_(x), mouseY_(y) {}

    float getX() const { return mouseX_; }
    float getY() const { return mouseY_; }

    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float mouseX_, mouseY_;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset)
        : xOffset_(xOffset), yOffset_(yOffset) {}

    float getXOffset() const { return xOffset_; }
    float getYOffset() const { return yOffset_; }

    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

private:
    float xOffset_, yOffset_;
};

class MouseButtonEvent : public Event {
public:
    MouseButton getMouseButton() const { return button_; }

    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

protected:
    MouseButtonEvent(MouseButton button) : button_(button) {}
    MouseButton button_;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    MouseButtonPressedEvent(MouseButton button) : MouseButtonEvent(button) {}

    EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    MouseButtonReleasedEvent(MouseButton button) : MouseButtonEvent(button) {}

    EVENT_CLASS_TYPE(MouseButtonReleased)
};

// Event dispatcher
class EventDispatcher {
public:
    EventDispatcher(Event& event) : event_(event) {}

    template<typename T, typename F>
    bool dispatch(const F& func) {
        if (event_.getEventType() == T::getStaticType()) {
            event_.handled = func(static_cast<T&>(event_));
            return true;
        }
        return false;
    }

private:
    Event& event_;
};

} // namespace Perceptral
