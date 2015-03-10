/*
 * StringUtilTest.cpp
 *
 *  Created on: 02.03.2015
 *      Author: jonas
 */

#include <gmock/gmock.h>
#include <locale>
#include "./StringUtil.h"

using ::testing::ElementsAre;

using StringUtil::startswith;
using StringUtil::endswith;

TEST(StringUtilTest, startswith)
{
    EXPECT_TRUE(startswith("abcd", "abc"));
    EXPECT_TRUE(startswith("abcd", ""));
    EXPECT_FALSE(startswith("abcd", "ABC"));
    EXPECT_FALSE(startswith("abcd", "efgh"));
    EXPECT_FALSE(startswith("abcd", "abce"));
    EXPECT_FALSE(startswith("abcd", "abcde"));
}

TEST(StringUtilTest, endswith)
{
    EXPECT_TRUE(endswith("abcd", "bcd"));
    EXPECT_TRUE(endswith("abcd", ""));
    EXPECT_FALSE(endswith("abcd", "BCD"));
    EXPECT_FALSE(endswith("abcd", "efgh"));
    EXPECT_FALSE(endswith("abcd", "ebcd"));
    EXPECT_FALSE(endswith("abcd", "cde"));
}

TEST(StringUtilTest, utf8to32)
{
    std::wstring result = StringUtil::utf8to32("Ä, ö, ü und ß.");
    EXPECT_EQ(L"Ä, ö, ü und ß.", result);
}

TEST(StringUtilTest, utf32to8)
{
    std::string result = StringUtil::utf32to8(L"Ä, ö, ü und ß.");
    EXPECT_EQ("Ä, ö, ü und ß.", result);
}

TEST(StringUtilTest, string_isalnum)
{
    std::string s = "hä?";
    EXPECT_EQ(4, s.size());
    EXPECT_TRUE(isalnum(s[0]));
    EXPECT_FALSE(isalnum(s[1]));
    EXPECT_FALSE(isalnum(s[3]));
}

TEST(StringUtilTest, wstring_isalnum)
{
    std::wstring s = L"hä?";
    std::locale loc("de_DE.UTF8");
    EXPECT_EQ(3, s.size());
    EXPECT_TRUE(std::isalnum(s[0]));
    EXPECT_FALSE(std::isalnum(s[1]));
    EXPECT_TRUE(std::isalnum(s[1], loc));
    EXPECT_FALSE(std::isalnum(s[2]));
}

TEST(StringUtilTest, split_string)
{
    std::vector<std::string> result = StringUtil::split_string("Hi you there!");
    EXPECT_THAT(result, ElementsAre("Hi", "you", "there!"));
}

TEST(StringUtilTest, wstring_switch_case)
{
    std::wstring str = L"Äöüß";
    std::locale loc("de_DE.UTF8");
    std::transform(str.begin(), str.end(), str.begin(),
                   [loc](wchar_t x){return std::tolower(x, loc);});
    EXPECT_EQ(L"äöüß", str);
    std::transform(str.begin(), str.end(), str.begin(),
                   [loc](wchar_t x){return std::toupper(x, loc);});
    EXPECT_EQ(L"ÄÖÜß", str);
}

TEST(StringUtilTest, DISABLED_string_switch_case)
{
    std::string str = "Äöüß";
    std::locale loc("de_DE.UTF8");
    // NOTE(Jonas): This does not work, because the transformation is
    // character-wise and the the German Umlaute take more than one
    // character in unicode/utf8.
    std::transform(str.begin(), str.end(), str.begin(),
                   [loc](char x){return std::tolower(x, loc);});
    std::cout << str << std::endl;
    EXPECT_EQ("äöüß", str);
    std::transform(str.begin(), str.end(), str.begin(),
                   [loc](char x){return std::toupper(x, loc);});
    std::cout << str << std::endl;
    EXPECT_EQ("ÄÖÜß", str);
}

TEST(StringUtilTest, u_tolower)
{
    std::string str = "ÄÖÜß";
    EXPECT_EQ("äöüß", StringUtil::u_tolower(str));
}
