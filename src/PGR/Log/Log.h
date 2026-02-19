#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <cstdarg>
#include <chrono>
#include <windows.h>

enum class LogLevel {
    Debug,
    Info,
    Notice,
    Warning,
    Error,
    Fatal
};

std::string getCurrentTime();
std::string logLevelToString(LogLevel level);
std::string getColoredLogLevel(LogLevel level);
void log(LogLevel level, const char* file, int line, const char* func, const std::string format, ...);

LogLevel getLogLevel();
void setLogLevel(LogLevel level);
void setLogEnd(const std::string& end = "\n");

#define LogDebug(format, ...)   log(LogLevel::Debug, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogInfo(format, ...)    log(LogLevel::Info, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogNotice(format, ...)  log(LogLevel::Notice, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogWarning(format, ...) log(LogLevel::Warning, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogError(format, ...)   log(LogLevel::Error, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogFatal(format, ...)   log(LogLevel::Fatal, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
