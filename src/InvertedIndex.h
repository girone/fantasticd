#ifndef INVERTEDINDEX_H_
#define INVERTEDINDEX_H_

#include <gtest/gtest_prod.h>
#include <locale>
#include <unordered_map>
#include <utility>
#include <vector>


struct PairHash
{
    template<class U, class V>
    size_t operator()(const std::pair<U, V>& p) const
    {
        return std::hash<U>(p.first) ^ std::hash<V>(p.second);
    }
};

typedef std::pair<std::string, size_t> KW;

// A lexicographical comparator.
auto lexicographycally = [](const KW& a, const KW& b) {
    return std::lexicographical_compare(a.first.begin(), a.first.end(),
                                        b.first.begin(), b.first.end());
};


// A by-importance comparator.
auto by_importance = [](const KW& a, const KW& b) {
    return a.second > b.second;
};


class InvertedIndex
{
public:
    typedef unsigned int DocumentID;
    typedef std::string ICDcode;
    typedef unsigned int ICDcodeIndex;
    struct Entry
    {
        Entry()
            : document_id(-1), code_index(-1), score(-1) { }
        Entry(DocumentID d, ICDcodeIndex c, float s)
            : document_id(d), code_index(c), score(s) { }
        bool operator==(const Entry& other) const
        {
            return document_id == other.document_id &&
                   code_index == other.code_index &&
                   score == other.score;
        }
        DocumentID document_id;
        ICDcodeIndex code_index;
        float score;
    };

    void create_from_ICD_HTML(const std::string& directory);
    void parse_ICD_HTML_file(const std::string& filepath);
    void index_word(const std::string& word, DocumentID document_index, ICDcodeIndex code_index);
    // Returns true, if the word or token should not be indexed.
    bool ignore(const std::string& word);

    std::vector<Entry> search(const std::string& keyword, const size_t* top_k=NULL) const;
    std::vector<Entry> search(const std::vector<std::string>& keywords, const size_t* top_k=NULL) const;

    // Returns a list of suggestions for auto-completion.
    // Suggestions have a prefix edit distance of at most delta.
    // May be limited to the top k matches.
    std::vector<std::string> suggest(
            const std::string& keyword_prefix,
            const unsigned int* delta=NULL,
            const unsigned int* top_k=NULL) const;

    static std::vector<Entry> intersection(const std::vector<Entry>& list1, const std::vector<Entry>& list2);
    static std::vector<Entry> andish_union(const std::vector<Entry>& list1, const std::vector<Entry>& list2);

    // Returns a vector of elements in the union of all input lists and
    // a counter indicating the number of duplicates. Expects the input
    // lists to be sorted.
    static std::vector<std::pair<size_t, size_t>> union_with_counter(
            const std::vector<const std::vector<size_t>* >& lists);

    // Bring the search result into different formats.
    std::vector<std::string> format_search_result(const std::vector<Entry>& result) const;
    std::vector<std::string> format_search_result_items(const std::vector<Entry>& results) const;
    std::string format_id(const ICDcode& icd_code) const;

private:
    std::unordered_map<std::string, std::vector<Entry>> index_;
    std::vector<std::string> documents_;
    // Maps from document, code index to the actual ICD code, e.g. "S83.53".
    std::vector<ICDcode> icd_codes_;
    std::unordered_map<ICDcode, ICDcodeIndex> code_to_code_index_;
    // A lexicographically sorted list of pairs (keyword, importance).
    std::vector<std::pair<std::string, float>> keywords_;
    // A index from q-grams of keywords to keywords. Used for error-tolerant prefix search to suggest input.
    typedef std::string QGram;
    typedef size_t QGramEntry;
    std::unordered_map<QGram, std::vector<QGramEntry>> keyword_index_;

    // tf.idf related stuff
    std::unordered_map<ICDcodeIndex, size_t> number_of_words_;
    size_t sum_of_document_lengths_;

    static const std::locale LOCALE;
    static const std::string DIMDI_ICD_URL_prefix;
    static const unsigned int Q_GRAM_LENGTH;
    static const char Q_GRAM_PADDING_CHAR;

    // Computes the bm25 weights of each entry.
    void compute_ranking_scores();
    // Computes the keywords and their score.
    void compute_keyword_importances();
    // Computes the index over keywords.
    void compute_keyword_index(unsigned int q);

    FRIEND_TEST(InvertedIndexTest, parse_ICD_from_HTML);
    FRIEND_TEST(InvertedIndexTest, suggest);
    FRIEND_TEST(InvertedIndexTest, compute_keyword_importances);
    FRIEND_TEST(InvertedIndexTest, compute_keyword_index);

    // TODO(Jonas): The indexes may be stored compressed, if space becomes an issue.
};


#endif  // INVERTEDINDEX_H_
