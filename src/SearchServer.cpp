/*
 * SearchServer.cpp
 *
 *  Created on: 11.03.2015
 *      Author: jonas
 */

#include "./SearchServer.h"
#include <fstream>
#include "./StringUtil.h"

using boost::asio::ip::tcp;

// Turns exception handling in the run loop off. Useful for debugging.
// #define NO_EXCEPTIONS

void SearchServer::run()
{
    while (true)
    {
#ifndef NO_EXCEPTIONS
        try
        {
#endif
            std::cout << "Waiting for query at port " << port_ << "..." << std::endl;
            tcp::socket client(io_service_);
            acceptor_.accept(client);

            // Extract the query string.
            std::vector<char> request_buffer(1024);
            boost::system::error_code read_error, write_error;
            client.read_some(boost::asio::buffer(request_buffer), read_error);
            std::string request(request_buffer.size(), 0);
            std::copy(request_buffer.begin(), request_buffer.end(), request.begin());
            size_t pos = request.rfind('\r');
            if (pos != std::string::npos)
            {
                request.resize(pos);
            }

            std::cout << "Request string is \"" << request << "\"." << std::endl;
            if (not StringUtil::startswith(request, "GET"))
            {
                std::cout << "I do only handle GET requests. Skipping request." << std::endl;
                continue;
            }

            std::string response = compute_HTTP_response(request);
            boost::asio::write(client, boost::asio::buffer(response),
                    boost::asio::transfer_all(), write_error);
#ifndef NO_EXCEPTIONS
        }
        catch (const std::exception& e)
        {
            std::cerr << "Exception thrown: " << e.what() << std::endl;
        }
#endif
    }
}

std::string SearchServer::compute_HTTP_response(const std::string& request) const
{
    // Extract URL from "GET /<url> HTTP/1.1 ...".
    size_t end_of_URL = request.find(' ', 5);
    std::string filename = "www/" + request.substr(5, end_of_URL - 5);

    std::string response;
    std::ifstream ifs(filename);
    if (!ifs.good())
    {
        std::cerr << "Error: File \"" << filename << "\" not found." << std::endl;
        response = "Not found.";
    }
    else
    {
        // Read in the whole file.
        // More efficient than line by line reading according to SO:
        // http://stackoverflow.com/a/2602258/841567
        ifs.seekg(0, std::ios::end);
        size_t filesize = ifs.tellg();
        response.assign(filesize, ' ');
        ifs.seekg(0, std::ios::beg);
        ifs.read(&response[0], filesize);
    }
    std::cout << "Response is: \"" << response << "\"" << std::endl;
    return response;
}
