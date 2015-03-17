/*
 * InvertedIndex.cpp
 *
 *  Created on: 02.03.2015
 *      Author: jonas
 */
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "./InvertedIndex.h"
#include "./StringUtil.h"

using std::string;
using std::wstring;
using std::vector;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::filesystem::directory_iterator;


// TODO(Jonas): This binds the code to German ICD input. Maybe it works on
// English input (ASCII), too. But if other languages shall be supported, this
// has to be parameterized.
const std::locale InvertedIndex::LOCALE = std::locale("de_DE.UTF8");

const std::string InvertedIndex::DIMDI_ICD_URL_prefix =
        "http://www.dimdi.de/static/de/klassi/icd-10-gm/kodesuche/onlinefassungen/htmlgm2015/";

void InvertedIndex::create_from_ICD_HTML(const string& directory)
{
    sum_of_document_lengths_ = 0;
    boost::filesystem::path p(directory);
    assert(exists(p) && is_directory(p));
    vector<string> filenames;
    for (directory_iterator it = directory_iterator(p); it != directory_iterator(); ++it)
    {
        if (not StringUtil::startswith(it->path().filename().string(), ".")
            and StringUtil::endswith(it->path().string(), ".htm"))
        {
           filenames.push_back(it->path().string());
        }
    }
    std::cout << "Creating ICD index from " << filenames.size() << " file(s)." << std::endl;
    for (string s: filenames)  // DEBUG
        std::cout << s << std::endl;

    // Parse the index from HTML files.
    for (const string& file: filenames)
    {
        std::cout << "Parsing " << file << "..." << std::endl;
        parse_ICD_HTML_file(file);
    }

    // Compute the weights (BM25).
    compute_ranking_scores();

    compute_keyword_importances();
}

void InvertedIndex::parse_ICD_HTML_file(const string& filepath)
{
    std::regex re("[A-Z][0-9]+\\.[0-9]*-*");
    bool indexing_active = false;
    bool inside_tag = false;
    DocumentID document_index = documents_.size();
    ICDcode icd_code = "";
    ICDcodeIndex code_index = -1;
    std::ifstream ifs(filepath);
    assert(ifs.is_open());
    string rawline;
    while (getline(ifs, rawline))
    {
        // Convert to utf32 for robust isalnum().
        wstring line = StringUtil::utf8to32(rawline);
        size_t pos = 0;
        while (pos < line.size())
        {
            if (inside_tag && line[pos] == '>')
            {
                inside_tag = false;
                ++pos;
            }
            else if (inside_tag || line[pos] == '<')
            {
                inside_tag = true;
                for (; pos < line.size() && line[pos] != '>'; ++pos);
            }
            else if (isspace(line[pos]))
            {
                // Skip until next non-space.
                for (; pos < line.size() && isspace(line[pos]); ++pos);
            }
            else if (isalnum(line[pos], LOCALE))
            {
                // Check for ICD codes.
                size_t pos2 = pos + 1;
                for (; pos2 < line.size() && line[pos2] != '<'; ++pos2);
                if (line[pos2] == '<' && pos2 > pos + 2 && pos2 <= pos + 7 && line[pos2] == '<')
                {
                    std::string candidate = StringUtil::utf32to8(line.substr(pos, pos2 - pos));
                    if (std::regex_match(candidate, re))
                    {
                        // The first valid ICD code enables the indexing:
                        indexing_active = true;

                        icd_code = StringUtil::tolower(candidate);
                        auto it = code_to_code_index_.find(icd_code);
                        if (it == code_to_code_index_.end())
                        {
                            code_index = icd_codes_.size();
                            icd_codes_.push_back(icd_code);
                            code_to_code_index_[icd_code] = code_index;
                        }
                        else
                        {
                            code_index = it->second;
                        }
                        index_word(icd_code, document_index, code_index);
                        pos = pos2;
                        continue;
                    }
                }

                // Find end of word and index it.
                size_t start_of_word = pos;
                for (; pos < line.size() && isalnum(line[pos], LOCALE); ++pos);
                wstring wword = line.substr(start_of_word, pos - start_of_word);
                string word = StringUtil::utf32to8(StringUtil::u_tolower(wword));
                if (!ignore(word) and indexing_active)
                {
                    index_word(word, document_index, code_index);
                }
            }
            else
            {
                // non-alphanumeric character
                ++pos;
            }
        }
    }
    boost::filesystem::path p(filepath);
    documents_.push_back(p.filename().string());
}

void InvertedIndex::index_word(const string& word, DocumentID document_index, ICDcodeIndex code_index)
{
    auto it = index_.find(word);
    if (it == index_.end() || it->second.back().code_index != code_index)
    {
        index_[word].emplace_back(document_index, code_index, 1);
    }
    else
    {
        index_[word].back().score++;  // Increase term frequency count.
    }
    // Keep track of the "document length".
    number_of_words_[code_index] += word.size();
    sum_of_document_lengths_ += word.size();
}

bool InvertedIndex::ignore(const string& word)
{
    // Ignore HTML tags.
    if (StringUtil::startswith(word, "<"))
    {
        return true;
    }
    return false;
}

vector<InvertedIndex::Entry> InvertedIndex::search(
        const std::string& keyword,
        const size_t* top_k)
const
{
    std::vector<std::string> tmp = {keyword};
    return search(tmp, top_k);
}

vector<InvertedIndex::Entry> InvertedIndex::search(
        const vector<string>& keywords,
        const size_t* top_k)
const
{
    if (keywords.size() == 0)
    {
        return vector<Entry>();
    }
    vector<const vector<Entry>*> inverted_lists;
    inverted_lists.reserve(keywords.size());
    for (const string& keyword: keywords)
    {
        auto it = index_.find(StringUtil::u_tolower(keyword));
        if (it != index_.cend())
        {
            inverted_lists.push_back(&(it->second));
        }
    }
    if (inverted_lists.size() == 0)
    {
        return vector<Entry>();
    }

    // Sort by length to speed intersection up.
    auto comp = [](const vector<Entry>* a, const vector<Entry>* b) {
        return a->size() < b->size();
    };
    std::sort(inverted_lists.begin(), inverted_lists.end(), comp);

    // Intersect or unite.
    vector<Entry> result = *(inverted_lists[0]);
    for (size_t i = 1; i < inverted_lists.size(); ++i)
    {
        result = intersection(result, *inverted_lists[i]);
    }

    // Sort by score.
    auto compare_score = [](const Entry& a, const Entry& b) {
        return a.score > b.score;
    };
    if (top_k and *top_k < result.size())
    {
        size_t k = *top_k;
        std::partial_sort(result.begin(), result.begin() + k, result.end(), compare_score);
    }
    else
    {
        std::sort(result.begin(), result.end(), compare_score);
    }
    return result;
}

std::vector<InvertedIndex::Entry> InvertedIndex::intersection(
        const std::vector<Entry>& list1,
        const std::vector<Entry>& list2)
{
    std::vector<Entry> result;
    result.reserve(std::min(list1.size(), list2.size()));
    std::vector<Entry>::const_iterator it1, it2;
    for (it1 = list1.cbegin(), it2 = list2.cbegin();
         it1 != list1.cend() && it2 != list2.cend();
         ++it1, ++it2)
    {
        for (; it1 != list1.cend() && it1->code_index < it2->code_index; ++it1);

        for (; it2 != list2.cend() && it2->code_index < it1->code_index; ++it2);

        if (it1 != list1.cend() && it2 != list2.cend() &&
            it1->code_index == it2->code_index)
        {
            result.emplace_back(
                    it1->document_id,
                    it1->code_index,
                    it1->score + it2->score
            );
        }
    }
    return result;
}

std::vector<InvertedIndex::Entry> InvertedIndex::andish_union(
        const std::vector<Entry>& list1,
        const std::vector<Entry>& list2)
{
    std::vector<Entry> result;
    std::vector<Entry>::const_iterator it1, it2;
    for (it1 = list1.cbegin(), it2 = list2.cbegin();
         it1 != list1.cend() || it2 != list2.cend();)
    {
        for (;
             it1 != list1.cend() &&
             (it2 == list2.cend() || it1->code_index < it2->code_index);
             ++it1)
        {
            result.emplace_back(
                    it1->document_id,
                    it1->code_index,
                    it1->score);
        }
        for (;
             it2 != list2.cend() &&
             (it1 == list1.cend() || it2->code_index < it1->code_index);
             ++it2)
        {
            result.emplace_back(
                    it2->document_id,
                    it2->code_index,
                    it2->score);
        }
        for (; it1 != list1.cend() && it2 != list2.cend() &&
               it1->code_index == it2->code_index;
             ++it1, ++it2)
        {
            result.emplace_back(
                    it1->document_id,
                    it1->code_index,
                    it1->score + it2->score);
        }
    }

    return result;
}

std::string InvertedIndex::format_id(const ICDcode& icd_code) const
{
    ICDcode id = StringUtil::toupper(icd_code);
    if (id.back() == '-')
    {
        id = id.substr(0, id.size() - 1);
    }
    if (id.back() == '.')
    {
        id = id.substr(0, id.size() - 1);
    }
    return id;
}

std::vector<std::string> InvertedIndex::format_search_result(
        const std::vector<Entry>& result) const
{
    std::vector<std::string> formatted;
    for (const auto& entry: result)
    {
        const std::string& document = documents_[entry.document_id];
        std::string id = format_id(icd_codes_[entry.code_index]);
        std::string url = DIMDI_ICD_URL_prefix + document + "#" + id;
        std::string score = convert<std::string>(entry.score);
        formatted.push_back("(" + url + ")," + "score=" + score);
    }
    return formatted;
}

void InvertedIndex::compute_ranking_scores()
{
    const float k = 1.75;
    const float b = 0.75;
    float average_document_length = 1.f * sum_of_document_lengths_ / icd_codes_.size();

    for (auto it = index_.begin(); it != index_.end(); ++it)
    {
        for (Entry& e: it->second)
        {
            ICDcodeIndex code_index = e.code_index;
            size_t document_length = number_of_words_[code_index];
            size_t tf = e.score;
            e.score = tf * (k + 1) /
                    (k * (1 - b + b * document_length / average_document_length) + tf);
        }
    }

}

void InvertedIndex::compute_keyword_importances()
{
    // TODO(Jonas): In the keyword list, the casing should be correct upper- or
    // lowercase, not unified lowercase as in the index. However, the search
    // should be case-insensitive.
    // Collect all keywords and scores.
    keywords_.clear();
    keywords_.reserve(index_.size());
    for (auto it = index_.cbegin(); it != index_.cend(); ++it)
    {
        keywords_.emplace_back(it->first, 1.0);
    }
    // Sort.
    std::sort(keywords_.begin(), keywords_.end(), lexicographycally);
}

std::vector<std::string> InvertedIndex::suggest(const std::string& keyword_prefix, const unsigned int* top_k) const
{
    assert(keyword_prefix.size() > 0);
    // Extract matches.
    std::pair<std::string, float> key(keyword_prefix, 1.0);
    auto lower = std::lower_bound(keywords_.begin(), keywords_.end(), key, lexicographycally);
    key.first.back() += 1;  // A hack to get upper bound to work.
    auto upper = std::upper_bound(keywords_.begin(), keywords_.end(), key, lexicographycally);
    std::vector<std::pair<std::string, float>> keywords(lower, upper);
    // Sort by importance.
    if (top_k && *top_k < keywords.size())
    {
        std::partial_sort(keywords.begin(), keywords.begin() + *top_k, keywords.end(), by_importance);
        keywords.resize(*top_k);
    }
    else
    {
        std::sort(keywords.begin(), keywords.end(), by_importance);
    }
    // Return only keywords.
    auto selector = [](const std::pair<std::string, float>& k_and_s) {
        return k_and_s.first;
    };
    std::vector<std::string> suggestions;
    suggestions.reserve(keywords.size());
    std::transform(keywords.begin(), keywords.end(), std::back_inserter(suggestions), selector);
    return suggestions;
}

