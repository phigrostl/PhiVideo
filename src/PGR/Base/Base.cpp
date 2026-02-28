#include "Base.h"

void Exita(std::string message, const int code, const char* file, int line, const char* func) {
    setLogLevel(LogLevel::Debug);

    if (code != 0) {
        log(LogLevel::Fatal, file, line, func, message);
        log(LogLevel::Notice, file, line, func, "Exiting with code: %d", code);
    }
    else
        log(LogLevel::Notice, file, line, func, message);
    exit(code);
}

std::string wstr2str(const std::wstring & wstr) {
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string str(bufferSize, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], bufferSize, nullptr, nullptr);
    return str;
}

std::wstring str2wstr(const std::string& str) {
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], bufferSize);
    return wstr;
}

std::string utf82gbk(const std::string& str) {
    int wide_len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring wide_str(wide_len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wide_str[0], wide_len);
    int gbk_len = WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), static_cast<int>(wide_str.size()), nullptr, 0, nullptr, nullptr);
    std::string gbk_str(gbk_len, '\0');
    WideCharToMultiByte(CP_ACP, 0, wide_str.c_str(), static_cast<int>(wide_str.size()), &gbk_str[0], gbk_len, nullptr, nullptr);

    return gbk_str;
}

std::string gbk2utf8(const std::string& str) {
    int wide_len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring wide_str(wide_len, 0);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.size()), &wide_str[0], wide_len);
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), static_cast<int>(wide_str.size()), nullptr, 0, nullptr, nullptr);
    std::string utf8_str(utf8_len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), static_cast<int>(wide_str.size()), &utf8_str[0], utf8_len, nullptr, nullptr);

    return utf8_str;
}
