#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace Perceptral {

class Log {
public:
    static void init();
    static void shutdown();

    // Set log level at runtime
    static void setLevel(spdlog::level::level_enum level);

    // Convenience methods for common log levels
    static void setVerbose(bool verbose);
    static void setTrace() { setLevel(spdlog::level::trace); }
    static void setDebug() { setLevel(spdlog::level::debug); }
    static void setInfo() { setLevel(spdlog::level::info); }
    static void setWarn() { setLevel(spdlog::level::warn); }
    static void setError() { setLevel(spdlog::level::err); }

    inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
    inline static std::shared_ptr<spdlog::logger>& getLogger() { return s_Logger; }

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_Logger;
};

} // namespace Perceptral

// Core logging macros
#define PC_CORE_TRACE(...)    ::Perceptral::Log::getCoreLogger()->trace(__VA_ARGS__)
#define PC_CORE_DEBUG(...)    ::Perceptral::Log::getCoreLogger()->debug(__VA_ARGS__)
#define PC_CORE_INFO(...)     ::Perceptral::Log::getCoreLogger()->info(__VA_ARGS__)
#define PC_CORE_WARN(...)     ::Perceptral::Log::getCoreLogger()->warn(__VA_ARGS__)
#define PC_CORE_ERROR(...)    ::Perceptral::Log::getCoreLogger()->error(__VA_ARGS__)
#define PC_CORE_CRITICAL(...) ::Perceptral::Log::getCoreLogger()->critical(__VA_ARGS__)

#define PC_TRACE(...)    ::Perceptral::Log::getLogger()->trace(__VA_ARGS__)
#define PC_DEBUG(...)    ::Perceptral::Log::getLogger()->debug(__VA_ARGS__)
#define PC_INFO(...)     ::Perceptral::Log::getLogger()->info(__VA_ARGS__)
#define PC_WARN(...)     ::Perceptral::Log::getLogger()->warn(__VA_ARGS__)
#define PC_ERROR(...)    ::Perceptral::Log::getLogger()->error(__VA_ARGS__)
#define PC_CRITICAL(...) ::Perceptral::Log::getLogger()->critical(__VA_ARGS__)


#ifdef PC_ENABLE_ASSERTS
    #ifdef _MSC_VER
        #define PC_DEBUGBREAK() __debugbreak()
    #elif defined(__linux__) || defined(__APPLE__)
        #include <signal.h>
        #define PC_DEBUGBREAK() raise(SIGTRAP)
    #else
        #define PC_DEBUGBREAK()
    #endif

    #define PC_CORE_ASSERT(x, ...) \
        { if(!(x)) { PC_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); PC_DEBUGBREAK(); } }
    
    #define PC_ASSERT(x, ...) \
        { if(!(x)) { PC_ERROR("Assertion Failed: {0}", __VA_ARGS__); PC_DEBUGBREAK(); } }
#else
    #define PC_CORE_ASSERT(x, ...)
    #define PC_ASSERT(x, ...)
#endif
