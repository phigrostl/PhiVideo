#pragma once

#include <iostream>
#include <string>
#include <Windows.h>

void Exit(std::string message, const int code);

std::string wstr2str(const std::wstring& wstr);
std::wstring str2wstr(const std::string& str);
