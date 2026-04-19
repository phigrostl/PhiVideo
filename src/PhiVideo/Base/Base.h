#pragma once

#include <PhiVideo/Log/Log.h>

void Exita(const char* file, int line, const char* func, const char* format, const int code, ...);
std::string FfmpegBaseCmd();

std::string wstr2str(const std::wstring& wstr);
std::wstring str2wstr(const std::string& str);
std::string utf82gbk(const std::string& str);
std::string gbk2utf8(const std::string& str);

#define Exit(format, code, ...) Exita(__FILE__, __LINE__, __func__, format, code, ##__VA_ARGS__)
