/*
 * InvertedIndex.cpp
 *
 *  Created on: 02.03.2015
 *      Author: jonas
 */
#include <boost/filesystem.hpp>

#include <cassert>
#include <fstream>
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
        filenames.push_back(it->path().string());
    }
    for (string s: filenames)
        std::cout << s << std::endl;

    for (const string& file: filenames)
    {
        std::cout << "Parsing " << file << "..." << std::endl;
        parse_ICD_HTML_file(file);
    }
}

void InvertedIndex::parse_ICD_HTML_file(const string& filename)
{
    document_id document_index = documents_.size();
    std::ifstream ifs(filename);
    assert(ifs.is_open());
    string rawline;
    while (getline(ifs, rawline))
    {
    	// Convert to utf16 for robust isalnum().
    	wstring line = StringUtil::utf8to16(rawline);
        size_t pos = 0;
        bool inside_tag = false;
        while (pos < line.size())
        {
        	// Find the start of a token.
        	for (; pos < line.size() && line[pos] == ' '; ++pos);

        	if (line[pos] == '<' || inside_tag)
        	{
        		inside_tag = true;
        		// Skip till end of tag or end of line.
        		for (; pos < line.size() && line[pos] != '>'; ++pos);
        		if (line[pos] == '>')
				{
					inside_tag = false;
				}
        		else
        		{
        			break;
        		}
        	}

        	// Find the end of the word.
        	size_t start_of_word = pos;
        	for (; pos < line.size() && isalnum(line[pos], LOCALE); ++pos);

        	// Convert back to utf8 for compact storage.
        	wstring wword = line.substr(start_of_word, pos - start_of_word);
        	string word = StringUtil::utf16to8(StringUtil::u_tolower(wword));

        	// Add the word to the index.
        	if (!ignore(word))
        	{
        		index_word(word, document_index);
        	}


        	++pos;
        }
    }
    documents_.push_back(filename);
}

void InvertedIndex::index_word(const string& word, document_id document_index)
{
	index_[word].push_back(document_index);  // TODO(jonas): Avoid duplicates here.
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

vector<document_id> InvertedIndex::search(const vector<string>& keywords) const
{

    vector<document_id> result;
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

