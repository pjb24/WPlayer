#include "GlobalUtility.h"

// std::wstring 문자열을 std::string 으로 변경함
std::string ConvertWstringToString(const std::wstring& var)
{
	std::locale loc("");
	auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
	return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
}

// std::string 문자열을 std::wstring 으로 변경함
std::wstring ConvertStringToWstring(const std::string& var)
{
	std::locale loc("");
	auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
	return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
}
