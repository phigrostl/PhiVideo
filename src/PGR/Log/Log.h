#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

#include <ctime>
#include <cstdarg>

#include <Windows.h>

enum class LogLevel {
    Debug,
    Info,
    Notice,
    Warning,
    Error,
    Fatal
};

struct Language {
    std::string name;
    std::string file;
    std::vector<std::string> before;
    std::vector<std::string> after;

    void Init(const std::string& file);
    std::string Translate(const std::string& str);
};

void InitLog(const std::string& folder);
void SetLanguage(int code);
std::string GetLanguages();

std::string getCurrentTime();
std::string logLevelToString(LogLevel level);
std::string getColoredLogLevel(LogLevel level);
void log(LogLevel level, const char* file, int line, const char* func, const char* format, ...);

LogLevel getLogLevel();
void setLogLevel(LogLevel level);
void setLogEnd(const std::string& end = "\n");

#define LogDebug(format, ...)   log(LogLevel::Debug, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogInfo(format, ...)    log(LogLevel::Info, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogNotice(format, ...)  log(LogLevel::Notice, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogWarning(format, ...) log(LogLevel::Warning, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogError(format, ...)   log(LogLevel::Error, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
#define LogFatal(format, ...)   log(LogLevel::Fatal, __FILE__, __LINE__, __func__, format, ##__VA_ARGS__)
