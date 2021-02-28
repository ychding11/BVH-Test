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

#define Log(fmt, ...)  Logging::Logger()->info(fmt,__VA_ARGS__); 
#define Warn(fmt, ...) Logging::Logger()->warn(fmt,__VA_ARGS__); 
#define Err(fmt, ...)  Logging::Logger()->error(fmt,__VA_ARGS__); 

#endif
