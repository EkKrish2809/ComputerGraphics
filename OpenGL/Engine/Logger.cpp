#include  "Logger.h"

logger::logger(){}

void logger::init_logger(const std::string &log_filename, const std::string &shader_filename)
{
    logFile.open(log_filename, std::ios::out);
    if (!logFile.is_open())
    {
        std::cerr << "Error opening log file: " << log_filename << std::endl;
    }

    shaderFile.open(shader_filename, std::ios::out);
    if (!shaderFile.is_open())
    {
        std::cerr << "Error opening shader log file: " << shader_filename << std::endl;
    }
}

logger::~logger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }

    if (shaderFile.is_open())
    {
        shaderFile.close();
    }
}

/*
    * log function
    * @param level: LogType enum
    * @param message: string
    * @param ...: variable arguments
    
    It will take the message string and variable arguments and create a new string out of it as a formatted string.
*/

void logger::log(LogType level, const std::string message, ...) {
    va_list args;
    va_start(args, message);
    // char buffer[1024];
    std::lock_guard<std::mutex> guard(logMutex);
    if (logFile.is_open()) {
        logFile << "[" << get_timestamp() << "] [" << log_level_to_string(level) << "] " ;
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), message.c_str(), args);
        logFile << buffer << std::endl;
    } else {
        std::cerr << "[" << get_timestamp() << "] [" << log_level_to_string(level) << "] " ;
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), message.c_str(), args);
        std::cerr << buffer << std::endl;
    }
}

std::string logger::get_timestamp() {
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    return buf;
}

std::string logger::log_level_to_string(LogType level) {
    switch (level) {
    case LogType::_INFO_:
        return "INFO";
    case LogType::_WARNING_:
        return "WARNING";
    case LogType::_ERROR_:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}