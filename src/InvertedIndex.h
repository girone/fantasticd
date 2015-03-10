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
    typedef std::pair<DocumentID, ICDcodeIndex> Entry;

    void create_from_ICD_HTML(const std::string& directory);
    void parse_ICD_HTML_file(const std::string& filename);
    void index_word(const std::string& word, DocumentID document_index, ICDcodeIndex code_index);
    // Returns true, if the word or token should not be indexed.
    bool ignore(const std::string& word);

    std::vector<Entry> search(const std::string& keyword) const;
    std::vector<Entry> search(const std::vector<std::string>& keywords) const;
    std::vector<std::string> format_search_result(const std::vector<Entry>& result) const;

private:
    std::unordered_map<std::string, std::vector<Entry>> index_;
    std::vector<std::string> documents_;
    // Maps from document, code index to the actual ICD code, e.g. "S83.53".
    std::vector<ICDcode> icd_codes_;
    std::unordered_map<ICDcode, ICDcodeIndex> code_to_code_index_;

    static const std::locale LOCALE;

    FRIEND_TEST(InvertedIndexTest, parse_ICD_from_HTML);
};


#endif  // INVERTEDINDEX_H_
