/*
 * searchServerMain.cpp
 *
 *  Created on: 11.03.2015
 *      Author: jonas
 */

#include "./SearchServer.h"
#include "./InvertedIndex.h"
#include "./StringUtil.h"


int main(const int argc, const char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: ./searchServerMain <PORT> [<INDEX_FILE>]" << std::endl;
        return 0;
    }
    unsigned int port = convert<unsigned int>(argv[1]);
    std::string indexFile = "ICD.ii";
    if (argc == 3)
    {
        indexFile = argv[2];
    }
    InvertedIndex ii;
    std::cout << "Loading inverted index from file '" << indexFile << "'..." << std::endl;
    ii.load(indexFile);
    std::cout << "Done." << std::endl;

    SearchServer server(port, ii);
    server.run();
}


