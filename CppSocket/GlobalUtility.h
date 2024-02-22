#include "framework.h"

#ifndef GLOBALUTILITY_H
#define GLOBALUTILITY_H

#include <string>
#include <locale>

// sprintf 처럼 std::string 문자열을 만듦
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

// sprintf 처럼 std::wstring 문자열을 만듦
template<typename ... Args>
std::wstring wstring_format(const std::wstring& format, Args ... args)
{
	int size_s = std::swprintf(nullptr, 0, format.c_str(), args ...) + 2; // Extra space for '\0'
	if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
	std::swprintf(buf.get(), size, format.c_str(), args ...);
	return std::wstring(buf.get(), buf.get() + size - 2); // We don't want the '\0' inside
}

// std::wstring 문자열을 std::string 으로 변경함
std::string ConvertWstringToString(const std::wstring& var);

// std::string 문자열을 std::wstring 으로 변경함
std::wstring ConvertStringToWstring(const std::string& var);

#endif //GLOBALUTILITY_H
