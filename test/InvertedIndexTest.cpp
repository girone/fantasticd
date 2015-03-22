#include "./InvertedIndex.h"
#include <gmock/gmock.h>
#include <regex>
#include <fstream>
#include <utility>
#include "./StringUtil.h"

using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Not;
using std::make_pair;

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

class InvertedIndexTest : public ::testing::Test
{
public:
    InvertedIndexTest()
    {
        create_test_icd_html();
    }
};


TEST_F(InvertedIndexTest, ICD_code_regex)
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

TEST_F(InvertedIndexTest, parse_ICD_from_HTML)
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

TEST_F(InvertedIndexTest, tf_idf_score)
{
    InvertedIndex ii;
    ii.parse_ICD_HTML_file(test_file);
    // Correct counting of the term frequency.
    EXPECT_THAT(ii.search("doppelt"), ElementsAre(Entry(0, 2, 2)));
    EXPECT_THAT(ii.search("einfach"), ElementsAre(Entry(0, 2, 1)));

    // TODO(Jonas): Write a test for tf.idf formula with bm25.
}

TEST_F(InvertedIndexTest, compute_keyword_importances)
{
    InvertedIndex ii;
    ii.parse_ICD_HTML_file(test_file);
    ii.compute_keyword_importances();

    typedef std::pair<std::string, float> T;
    std::vector<T> expected = {
            {"s80.-", 1.0}, {"s82.2-", 1.0}, {"s82.21", 1.0},
            {"des", 1.0}, {"tibiaschaftes", 1.0}, {"mit", 1.0},
            {"fraktur", 1.0}, {"der", 1.0}, {"fibula", 1.0},
            {"jeder", 1.0}, {"teil", 1.0}, {"test", 1.0},
            {"ohne", 1.0}, {"anästhesie", 1.0}, {"einfach", 1.0},
            {"doppelt", 1.0}
    };
    std::sort(expected.begin(), expected.end(), lexicographycally);
    EXPECT_THAT(ii.keywords_, Eq(expected));
}

TEST_F(InvertedIndexTest, suggest)
{
    InvertedIndex ii;
    ii.parse_ICD_HTML_file(test_file);
    ii.compute_keyword_importances();
    ii.compute_keyword_index(InvertedIndex::Q_GRAM_LENGTH);

    EXPECT_THAT(ii.suggest("anästhesie"), Contains("anästhesie"));
    std::vector<std::string> suggestions = ii.suggest("f");
    std::cout << StringUtil::join(", ", suggestions) << std::endl;
    EXPECT_EQ(2, suggestions.size());
    EXPECT_THAT(suggestions, Contains("fraktur"));
    EXPECT_THAT(suggestions, Contains("fibula"));
    EXPECT_NE(ii.suggest("f"), ii.suggest("F"));

    suggestions = ii.suggest("tibia");
    std::cout << StringUtil::join(", ", suggestions) << std::endl;
    suggestions = ii.suggest("de");
    std::cout << StringUtil::join(", ", suggestions) << std::endl;
}

TEST_F(InvertedIndexTest, intersection)
{
    std::vector<Entry> a = {Entry(0, 0, 2.),                  Entry(0, 2, 2.)};
    std::vector<Entry> b = {Entry(0, 0, 2.), Entry(0, 1, 2.), Entry(0, 2, 1.5)};

    std::vector<Entry> r = InvertedIndex::intersection(a, b);
    EXPECT_THAT(r, ElementsAre(Entry(0, 0, 4.), Entry(0, 2, 3.5)));
}

TEST_F(InvertedIndexTest, andish_union)
{
    std::vector<Entry> a = {Entry(0, 0, 2.),                  Entry(0, 2, 2.)};
    std::vector<Entry> b = {Entry(0, 0, 2.), Entry(0, 1, 2.), Entry(0, 2, 1.5)};

    std::vector<Entry> r = InvertedIndex::andish_union(a, b);
    EXPECT_THAT(r, ElementsAre(Entry(0, 0, 4.), Entry(0, 1, 2.), Entry(0, 2, 3.5)));
}

TEST_F(InvertedIndexTest, compute_keyword_index)
{
    InvertedIndex ii;

    ii.parse_ICD_HTML_file(test_file);
    ii.compute_keyword_importances();
    unsigned int q = 3;
    ii.compute_keyword_index(3);

    int index_of_fibula = 5;
    ASSERT_EQ("fibula", ii.keywords_[index_of_fibula].first);

    EXPECT_THAT(ii.keyword_index_["$$f"], Contains(index_of_fibula));
    EXPECT_THAT(ii.keyword_index_["$fi"], Contains(index_of_fibula));
    EXPECT_THAT(ii.keyword_index_["fib"], Contains(index_of_fibula));
    EXPECT_THAT(ii.keyword_index_["ibu"], Contains(index_of_fibula));
    EXPECT_THAT(ii.keyword_index_["bul"], Contains(index_of_fibula));
    EXPECT_THAT(ii.keyword_index_["ula"], Contains(index_of_fibula));
}

TEST_F(InvertedIndexTest, union_with_counter)
{
    const std::vector<size_t> l1 = {1, 2, 3, 4};
    const std::vector<size_t> l2 = {1, 3, 5};
    const std::vector<size_t> l3 = {3, 4, 5, 6};
    const std::vector<size_t> l4 = {7};
    std::vector<const std::vector<size_t>* > input = {&l1, &l2, &l3, &l4};
    std::vector<std::pair<size_t, size_t>> u = InvertedIndex::union_with_counter(input);
    EXPECT_THAT(u, ElementsAre(
            make_pair(1, 2),
            make_pair(2, 1),
            make_pair(3, 3),
            make_pair(4, 2),
            make_pair(5, 2),
            make_pair(6, 1),
            make_pair(7, 1)
    ));
}

TEST_F(InvertedIndexTest, search)
{
    // TODO(Jonas): Adds tests for the search functionality "/?q=...".
}
