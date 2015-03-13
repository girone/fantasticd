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
        std::cout << "Usage: ./searchServerMain <PORT>" << std::endl;
        return 0;
    }
    unsigned int port = convert<unsigned int>(argv[1]);
    SearchServer server(port);
    server.run();
}


