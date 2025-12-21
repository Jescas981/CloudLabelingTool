#pragma once

namespace Perceptral {

/**
 * @brief Static utility class for querying application time
 * 
 * Provides platform-agnostic timing functionality.
 * Abstracts platform-specific timing (GLFW, chrono, etc.)
 */
class Time {
public:
    /**
     * @brief Get elapsed time since application start in seconds
     * @return Time in seconds as float
     */
    static double getTime();
    
    /**
     * @brief Get elapsed time since application start in milliseconds
     * @return Time in milliseconds as double
     */
    static double getTimeMilliseconds();
    
    /**
     * @brief Initialize the timing system
     * 
     * Should be called once during application startup.
     * Records the start time for relative time calculations.
     */
    static void initialize();

private:
    static double s_startTime;
};

} // namespace Perceptral