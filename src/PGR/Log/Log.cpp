#include "Log.h"

LogLevel g_log_level = LogLevel::Info;
std::string g_log_end = "\n";

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&local_time, "\033[92m%Y-%m-%d %H:%M:%S\033[0m");
    return oss.str();
}

std::string logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:   return "  [DEBUG]";
    case LogLevel::Info:    return "   [INFO]";
    case LogLevel::Notice:  return " [NOTICE]";
    case LogLevel::Warning: return "[WARNING]";
    case LogLevel::Error:   return "  [ERROR]";
    case LogLevel::Fatal:   return "  [FATAL]";
    default:                return "[UNKNOWN]";
    }
}

std::string getColoredLogLevel(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:   return "  [\033[32mDEBUG\033[0m]";
    case LogLevel::Info:    return "   [\033[34mINFO\033[0m]";
    case LogLevel::Notice:  return " [\033[36mNOTICE\033[0m]";
    case LogLevel::Warning: return "[\033[33mWARNING\033[0m]";
    case LogLevel::Error:   return "  [\033[31mERROR\033[0m]";
    case LogLevel::Fatal:   return "  [\033[35mFATAL\033[0m]";
    default:                return "[\033[32mUNKNOWN\033[0m]";
    }
}

char ls = '\0';

void log(LogLevel level, const char* file, int line, const char* func, const std::string format, ...) {
    if (level < g_log_level)
        return;

    try {
        const size_t buffer_size = 1024;
        char buffer[buffer_size] = { 0 };

        va_list va_args;
        va_start(va_args, format);
        vsnprintf(buffer, buffer_size - 1, format.c_str(), va_args);
        va_end(va_args);

        if (buffer[0] == '\r') {

            ls = '\r';

            std::ostringstream oss;

            oss << "\033[2K\r[" << getCurrentTime() << "]"
                << getColoredLogLevel(level)
                << "[\033[95m" << file << "\033[0m @ \033[93m" << func << "()\033[0m : \033[96m" << line << "\033[0m] "
                << std::string(buffer).substr(1);

            std::cout << oss.str();
        }
        else if (std::string(buffer).find('\n') != std::string::npos) {
            std::istringstream iss(buffer);
            std::vector<std::string> lines;
            std::string Line;

            while (std::getline(iss, Line))
                lines.push_back(Line);

            std::ostringstream oss1;
            std::ostringstream oss2;
            if (!lines.empty()) {
                oss2 << "\n\t" << lines[0] << "\n";
                for (int i = 1; i < lines.size(); i++)
                    oss2 << "\t" << lines[i] << "\n";
            }

            oss1 << "\n[" << getCurrentTime() << "]"
                << getColoredLogLevel(level)
                << "[\033[95m" << file << "\033[0m @ \033[93m" << func << "()\033[0m : \033[96m" << line << "\033[0m] "
                << oss2.str();

            std::cout << oss1.str() << g_log_end;
        }
        else {
            std::ostringstream oss;

            if (ls == '\r') {
                oss << "\n";
                ls = '\0';
            }

            oss << "[" << getCurrentTime() << "]"
                << getColoredLogLevel(level)
                << "[\033[95m" << file << "\033[0m @ \033[93m" << func << "()\033[0m : \033[96m" << line << "\033[0m] "
                << buffer;

            std::cout << oss.str() << g_log_end;
        }
        
    }
    catch (const std::exception& e) {
        std::cerr << "[" << getCurrentTime() << "]"
            << getColoredLogLevel(LogLevel::Error)
            << "[\033[95m" << file << "\033[0m @ \033[93m" << func << "()\033[0m : \033[96m" << line << "\033[0m] "
            << "Error while logging: " << e.what() << g_log_end;
    }
}

void setLogLevel(LogLevel level) {
    g_log_level = level;
}

void setLogEnd(const std::string& end) {
    g_log_end = end;
}
