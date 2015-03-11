#include "./InvertedIndex.h"
#include <gmock/gmock.h>
#include <regex>
#include <fstream>

using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::Not;

typedef InvertedIndex::Entry Entry;

const std::string test_file = "parser_input.TMP.htm";


void create_test_icd_html()
{
    std::ofstream tmp_file(test_file);
    tmp_file <<
        " </dd></dl></div><div class='Category1'><h4>       <a name='S80' id='S80' class='code'>S80.-</a>      \n"
        " <h5><a name='S82.2' id='S82.2' class='code'>S82.2-</a> \n"
        "      \n"
        "       <span class='label'>Fraktur des Tibiaschaftes</span></h5></div><div class='Category3'><h6>      <a name='S82.21' id='S82.21' class='code'>S82.21</a> \n"
        "\n"
        "               <span class='label'>Mit Fraktur der Fibula (jeder Teil) (Test: ohne Anästhesie) (einfach doppelt doppelt)</span></h6></div><div class='Category3'>\n";
    tmp_file.close();
}


TEST(InvertedIndexTest, ICD_code_regex)
{
    std::regex re("[A-Z][0-9]+\\.([0-9]*|-)");
    std::string subject = "S83.50";
    EXPECT_TRUE(std::regex_match(subject, re));
    EXPECT_TRUE(std::regex_match(subject.c_str() + 0, subject.c_str() + subject.size(), re));

    std::string subject2 = "S83.-";
    EXPECT_TRUE(std::regex_match(subject2, re));

    std::string subject3 = "Kreuzbandriss";
    EXPECT_FALSE(std::regex_match(subject3, re));
}

TEST(InvertedIndexTest, parse_ICD_from_HTML)
{
    InvertedIndex ii;
    ii.parse_ICD_HTML_file(test_file);

    EXPECT_THAT(ii.icd_codes_, ElementsAre("s80.-", "s82.2-", "s82.21"));
    EXPECT_THAT(ii.documents_, ElementsAre("parser_input.TMP.htm"));
    EXPECT_THAT(ii.index_["fibula"], ElementsAre(Entry(0, 2, 1)));
    EXPECT_EQ(ii.search(" "), std::vector<Entry>());
    EXPECT_EQ(ii.search(""), std::vector<Entry>());
    EXPECT_EQ(ii.search("\t"), std::vector<Entry>());
    EXPECT_THAT(ii.search("fraktur"), ElementsAre(Entry(0, 1, 1), Entry(0, 2, 1)));

    // Correct handling of unicode/utf8.
    EXPECT_THAT(ii.search("anästhesie"), ElementsAre(Entry(0, 2, 1)));
}

TEST(InvertedIndexTest, tf_idf_score)
{
    InvertedIndex ii;
    ii.parse_ICD_HTML_file(test_file);
    // Correct counting of the term frequency.
    EXPECT_THAT(ii.search("doppelt"), ElementsAre(Entry(0, 2, 2)));
    EXPECT_THAT(ii.search("einfach"), ElementsAre(Entry(0, 2, 1)));

    // TODO(Jonas): Write a test for tf.idf formula with bm25.
}

