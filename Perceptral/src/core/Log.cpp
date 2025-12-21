#include <Perceptral/core/Log.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Perceptral {

std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_Logger;

void Log::init()
{
    // Create color multi-threaded console logger
    s_CoreLogger = spdlog::stdout_color_mt("CLOUDCORE");
    s_Logger = spdlog::stdout_color_mt("APP");

    // Set default pattern: [timestamp] [logger name] [level] message
    spdlog::set_pattern("%^[%T] [%n] [%l] %v%$");

    // Set default log level to info
    s_CoreLogger->set_level(spdlog::level::info);
    s_Logger->set_level(spdlog::level::info);

    PC_CORE_INFO("Logger initialized");
}

void Log::shutdown()
{
    if (s_CoreLogger) {
        PC_CORE_INFO("Logger shutting down");
        s_CoreLogger->flush();
        spdlog::drop_all();
    }
}

void Log::setLevel(spdlog::level::level_enum level)
{
    if (s_CoreLogger) {
        s_CoreLogger->set_level(level);
        PC_CORE_DEBUG("Log level changed to: {}", spdlog::level::to_string_view(level));
    }
}

void Log::setVerbose(bool verbose)
{
    if (verbose) {
        setLevel(spdlog::level::trace);
    } else {
        setLevel(spdlog::level::info);
    }
}

} // namespace Perceptral
