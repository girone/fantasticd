#ifndef INVERTEDINDEX_H_
#define INVERTEDINDEX_H_

#include <locale>
#include <unordered_map>
#include <vector>

typedef unsigned int document_id;

class InvertedIndex
{
public:
    void create_from_ICD_HTML(const std::string& directory);
    void parse_ICD_HTML_file(const std::string& filename);
    void index_word(const std::string& word, document_id document_index);
    // Returns true, if the word or token should not be indexed.
    bool ignore(const std::string& word);

    std::vector<document_id> search(const std::vector<std::string>& keywords) const;

private:
    std::unordered_map<std::string, std::vector<document_id>> index_;
    std::vector<std::string> documents_;

    static const std::locale LOCALE;
};


#endif  // INVERTEDINDEX_H_
