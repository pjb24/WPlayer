#include "GlobalUtility.h"

// std::wstring ���ڿ��� std::string ���� ������
std::string ConvertWstringToString(const std::wstring& var)
{
	std::locale loc("");
	auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
	return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
}

// std::string ���ڿ��� std::wstring ���� ������
std::wstring ConvertStringToWstring(const std::string& var)
{
	std::locale loc("");
	auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
	return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
}
