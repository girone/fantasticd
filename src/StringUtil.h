#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <algorithm>
#include <string>
#include <sstream>

namespace StringUtil
{

bool startswith(const std::string& this_, const std::string& firstpart);

std::wstring utf8to16(const std::string& utf8);

std::string utf16to8(const std::wstring& utf16);

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
