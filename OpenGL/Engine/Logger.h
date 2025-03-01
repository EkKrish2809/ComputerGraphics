#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <stdarg.h>

class logger
{
public:
    enum class LogType
    {
        _INFO_,
        _WARNING_,
        _ERROR_
    };

    logger();
    ~logger();
    void init_logger(const std::string &log_filename, const std::string &shader_filename);
    void log(LogType type , const std::string message, ...);

private:
    std::ofstream logFile;
    std::ofstream shaderFile;
    std::mutex logMutex;

    std::string get_timestamp();
    std::string log_level_to_string(LogType level);
};

// logger g_logger;

#endif // LOGGER_H