#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <algorithm>
#include <string>
#include <sstream>

namespace StringUtil
{

bool startswith(const std::string& this_, const std::string& firstpart);

bool endswith(const std::string& this_, const std::string& lastpart);

// Converts utf8 to utf32.
std::wstring utf8to32(const std::string& utf8);

// Converts utf32 to utf8.
std::string utf32to8(const std::wstring& utf32);

template<class StringType>
StringType u_tolower(const StringType& unicode);

// Like Python's ",".join(iterable) this function joins iterable to a string.
template<class I>
std::string join(const std::string& connector, const I& iterable) {
    std::ostringstream os;
    for (auto it = iterable.begin(); it < iterable.end(); ++it)
    {
        os << (it != iterable.begin() ? connector : "") << *it;
    }
    return os.str();
}

std::vector<std::string> split_string(const std::string& input, const char separator=' ');

}  // namespace StringUtil


#endif  // STRINGUTIL_H_
