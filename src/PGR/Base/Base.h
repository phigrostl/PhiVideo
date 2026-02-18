#pragma once

#include <PGR/Log/Log.h>

#include <iostream>
#include <string>
#include <Windows.h>

void Exit(std::string message, const int code);

std::string wstr2str(const std::wstring& wstr);
std::wstring str2wstr(const std::string& str);
std::string utf82gbk(const std::string& str);
std::string gbk2utf8(const std::string& str);
