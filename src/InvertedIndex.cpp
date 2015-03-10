/*
 * InvertedIndex.cpp
 *
 *  Created on: 02.03.2015
 *      Author: jonas
 */
#include <boost/filesystem.hpp>

#include <cassert>
#include <fstream>
#include <regex>
#include <string>
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


void InvertedIndex::create_from_ICD_HTML(const string& directory)
{
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

    for (const string& file: filenames)
    {
        std::cout << "Parsing " << file << "..." << std::endl;
        parse_ICD_HTML_file(file);
    }
}

void InvertedIndex::parse_ICD_HTML_file(const string& filename)
{
    std::regex re("[A-Z][0-9]+\\.[0-9]*-*");
    bool inside_tag = false;
    DocumentID document_index = documents_.size();
    ICDcode icd_code = "";
    ICDcodeIndex code_index = -1;
    std::ifstream ifs(filename);
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
                if (!ignore(word))
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
    documents_.push_back(filename);
}

void InvertedIndex::index_word(const string& word, DocumentID document_index, ICDcodeIndex code_index)
{
    index_[word].emplace_back(document_index, code_index);  // TODO(jonas): Avoid duplicates here.
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

std::vector<InvertedIndex::Entry> InvertedIndex::search(const std::string& keyword) const
{
    std::vector<std::string> tmp = {keyword};
    return search(tmp);
}

vector<InvertedIndex::Entry> InvertedIndex::search(const vector<string>& keywords) const
{

    vector<Entry> result;
    // OR search
    for (const string& keyword: keywords)
    {
        auto it = index_.find(StringUtil::u_tolower(keyword));
        if (it != index_.cend())
        {
            std::copy(it->second.begin(), it->second.end(), std::back_inserter(result));
        }
    }

    return result;
}

std::vector<std::string> InvertedIndex::format_search_result(
        const std::vector<Entry>& result) const
{
    std::vector<std::string> formatted;
    for (const auto& entry: result)
    {
        const DocumentID& document_index = entry.first;
        const ICDcode& icd_code = icd_codes_[entry.second];
        formatted.push_back(documents_[document_index] + "#" + icd_code);
    }
    return formatted;
}
