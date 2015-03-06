#include "./StringUtil.h"
#include <boost/locale/encoding_utf.hpp>
#include <locale>
//#include <codecvt>  <-- not yet present in GCC
#include <string>

using boost::locale::conv::utf_to_utf;


namespace StringUtil
{

bool startswith(const std::string& this_, const std::string& firstpart)
{
    return std::equal(firstpart.begin(), firstpart.end(), this_.begin());
}

std::wstring utf8to16(const std::string& utf8)
{
	return utf_to_utf<wchar_t>(utf8.c_str(), utf8.c_str() + utf8.size());
}

std::string utf16to8(const std::wstring& utf16)
{
	return boost::locale::conv::utf_to_utf<char>(utf16.c_str(), utf16.c_str() + utf16.size());
}

std::vector<std::string> split_string(const std::string& input, const char separator)
{
	std::stringstream s;
	s << input;
	std::vector<std::string> result;
	std::string tmp;
	while (s.good())
	{
		s >> tmp;
		result.push_back(tmp);
	}
	return result;
}

}

