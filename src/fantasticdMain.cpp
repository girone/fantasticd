//============================================================================
// Name        : fantasticd.cpp
// Author      : Jonas Sternisko
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <string>
#include "./InvertedIndex.h"
#include "./StringUtil.h"

using std::string;

int main() {
    string command;

    InvertedIndex ii;
    ii.create_from_ICD_HTML("../icd/html/");
    while (true)
    {
        std::cout << "Enter search phrase:" << std::endl;
        std::cout << "> " << std::flush;
        std::getline(std::cin, command);
        std::vector<std::string> keywords = StringUtil::split_string(command);

        std::vector<document_id> document_ids = ii.search(keywords);
        std::cout << "Results are: [" << StringUtil::join(", ", document_ids) << "]"
                  << std::endl;

    }
    return 0;

}
