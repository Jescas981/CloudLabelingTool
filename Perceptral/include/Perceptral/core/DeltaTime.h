#pragma once

namespace Perceptral {

/**
 * @brief Wrapper class for delta time between frames
 * 
 * Provides convenient conversion between seconds and milliseconds.
 * Can be implicitly converted to float for ease of use.
 */
class DeltaTime {
public:
    /**
     * @brief Construct a DeltaTime object
     * @param time Time in seconds (default: 0.0f)
     */
    DeltaTime(float time = 0.0f)
        : m_time(time) {}
    
    /**
     * @brief Implicit conversion to float
     * @return Time in seconds
     */
    operator float() const { return m_time; }
    
    /**
     * @brief Get time in seconds
     * @return Time in seconds as float
     */
    float getSeconds() const { return m_time; }
    
    /**
     * @brief Get time in milliseconds
     * @return Time in milliseconds as float
     */
    float getMilliseconds() const { return m_time * 1000.0f; }

private:
    float m_time;  // Time in seconds
};

} // namespace Perceptral