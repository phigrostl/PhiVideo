#include "Base.h"

void Exit(std::string message, const int code) {
    std::cerr << message << std::endl;
    exit(code);
}

std::string wstr2str(const std::wstring & wstr) {
    std::string str(wstr.begin(), wstr.end());
    return str;
}

std::wstring str2wstr(const std::string& str) {
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring wstr(bufferSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], bufferSize);
    return wstr;
}
