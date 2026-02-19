#include "Log.h"

LogLevel g_log_level = LogLevel::Info;
std::string g_log_end = "\n";

size_t getTerminalWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return (size_t)csbi.srWindow.Right - (size_t)csbi.srWindow.Left + 1;
    }
    return 80;
}

int getVisibleLength(const std::string& str) {
    int length = 0;
    bool in_escape = false;
    
    for (char c : str) {
        if (c == '\033')
            in_escape = true;
        else if (in_escape && c == 'm')
            in_escape = false;
        else if (!in_escape)
            length++;
    }
    
    return length;
}

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream oss;
    int ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() % 1000;
    oss << std::put_time(&local_time, "\033[38;2;22;198;12m%Y-%m-%d %H:%M:%S.") << std::setw(3) << std::setfill('0') << ms << "\033[0m";
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
    case LogLevel::Debug:   return "  [\033[38;2;019;161;014mDEBUG\033[0m]";
    case LogLevel::Info:    return "   [\033[38;2;000;055;218mINFO\033[0m]";
    case LogLevel::Notice:  return " [\033[38;2;058;150;221mNOTICE\033[0m]";
    case LogLevel::Warning: return "[\033[38;2;193;156;000mWARNING\033[0m]";
    case LogLevel::Error:   return "  [\033[38;2;197;015;031mERROR\033[0m]";
    case LogLevel::Fatal:   return "  [\033[38;2;136;023;152mFATAL\033[0m]";
    default:                return "[\033[38;2;197;197;197mUNKNOWN\033[0m]";
    }
}

char ls = '\0';

void log(LogLevel level, const char* file, int line, const char* func, const std::string format, ...) {
    if (level < g_log_level) return;

    try {
        const size_t buffer_size = 1024;
        char buffer[buffer_size] = { 0 };

        va_list va_args;
        va_start(va_args, format);
        vsnprintf(buffer, buffer_size - 1, format.c_str(), va_args);
        va_end(va_args);

        if (strlen(buffer) == 0) return;
        for (int i = 0; i < strlen(buffer); i++) {
            if (buffer[i] != ' ' && buffer[i] != '\t' && buffer[i] != '\n' && buffer[i] != '\r')
                break;
            if (i == strlen(buffer) - 1)
                return;
        }

        if (buffer[0] == '\r') {

            ls = '\r';

            std::ostringstream oss;

            oss << "\033[2K\r[" << getCurrentTime() << "]"
                << getColoredLogLevel(level)
                << "[\033[38;2;180;000;158m" << file << "\033[0m @ \033[38;2;249;241;165m" << func << "()\033[0m : \033[38;2;097;214;214m" << line << "\033[0m] "
                << std::string(buffer).substr(1);

            std::string log_str = oss.str();
            int terminal_width = getTerminalWidth();
            int visible_length = getVisibleLength(log_str);
            if (visible_length >= terminal_width - 10) {
                std::string trimmed_str;
                int current_length = 0;
                bool in_escape = false;
                
                for (char c : log_str) {
                    if (c == '\033') {
                        in_escape = true;
                        trimmed_str += c;
                    } 
                    else if (in_escape && c == 'm') {
                        in_escape = false;
                        trimmed_str += c;
                    } 
                    else if (!in_escape) {
                        if (current_length >= terminal_width - 13)
                            break;
                        current_length++;
                        trimmed_str += c;
                    } 
                    else
                        trimmed_str += c;
                }
                
                log_str = trimmed_str + "...";
            }

            std::cout << log_str;
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
                << "[\033[38;2;180;000;158m" << file << "\033[0m @ \033[38;2;249;241;165m" << func << "()\033[0m : \033[38;2;097;214;214m" << line << "\033[0m] "
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
                << "[\033[38;2;180;000;158m" << file << "\033[0m @ \033[38;2;249;241;165m" << func << "()\033[0m : \033[38;2;097;214;214m" << line << "\033[0m] "
                << buffer;

            std::cout << oss.str() << g_log_end;
        }
        
    }
    catch (const std::exception& e) {
        std::cerr << "[" << getCurrentTime() << "]"
            << getColoredLogLevel(LogLevel::Error)
            << "[\033[38;2;180;000;158m" << file << "\033[0m @ \033[38;2;249;241;165m" << func << "()\033[0m : \033[38;2;097;214;214m" << line << "\033[0m] "
            << "Error while logging: " << e.what() << g_log_end;
    }
}

LogLevel getLogLevel() {
    return g_log_level;
}

void setLogLevel(LogLevel level) {
    g_log_level = level;
}

void setLogEnd(const std::string& end) {
    g_log_end = end;
}
