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

std::wstring utf8to32(const std::string& utf8)
{
    return utf_to_utf<wchar_t>(utf8.c_str(), utf8.c_str() + utf8.size());
}

std::string utf32to8(const std::wstring& utf32)
{
    return boost::locale::conv::utf_to_utf<char>(utf32.c_str(), utf32.c_str() + utf32.size());
}

template<>
std::wstring u_tolower(const std::wstring& unicode)
{
    std::wstring result;
    std::locale loc("de_DE.UTF8");
    std::transform(unicode.begin(), unicode.end(), std::back_inserter(result),
                   [loc](wchar_t x){return std::tolower(x, loc);});
    return result;
}

template<>
std::string u_tolower(const std::string& unicode)
{
    return utf32to8(u_tolower(utf8to32(unicode)));
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

