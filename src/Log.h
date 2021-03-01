#ifndef Log_h
#define Log_h

//
// https://github.com/gabime/spdlog
//
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace Logging
{
    spdlog::logger *Logger();
}

#define Log(...)  Logging::Logger()->info(__VA_ARGS__); 
#define Warn(...) Logging::Logger()->warn(__VA_ARGS__); 
#define Err(...)  Logging::Logger()->error(__VA_ARGS__); 

#endif
