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
    static std::vector<Entry> intersection(const std::vector<Entry>& list1, const std::vector<Entry>& list2);
    static std::vector<Entry> andish_union(const std::vector<Entry>& list1, const std::vector<Entry>& list2);
    std::vector<std::string> format_search_result(const std::vector<Entry>& result) const;
    std::string format_id(const ICDcode& icd_code) const;

private:
    std::unordered_map<std::string, std::vector<Entry>> index_;
    std::vector<std::string> documents_;
    // Maps from document, code index to the actual ICD code, e.g. "S83.53".
    std::vector<ICDcode> icd_codes_;
    std::unordered_map<ICDcode, ICDcodeIndex> code_to_code_index_;

    // tf.idf related stuff
    std::unordered_map<ICDcodeIndex, size_t> number_of_words_;
    size_t sum_of_document_lengths_;

    static const std::locale LOCALE;
    static const std::string DIMDI_ICD_URL_prefix;

    // Computes the bm25 weights of each entry.
    void compute_ranking_scores();

    FRIEND_TEST(InvertedIndexTest, parse_ICD_from_HTML);
};


#endif  // INVERTEDINDEX_H_
