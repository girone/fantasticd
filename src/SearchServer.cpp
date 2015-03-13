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
    std::string filename = request.substr(5, end_of_URL - 5);

    std::string response;
    std::ifstream ifs("www/" + filename);
    std::cout << "Looking for file \"" << filename << "\"." << std::endl;
    if (filename == "" || not ifs.good())
    {
        std::cerr << "Error: File \"" << filename << "\" not found." << std::endl;
        response = "Not found.";
    }
    else
    {
        std::string content_type = decide_content_type(filename);

        // Read in the whole file.
        // More efficient than line by line reading according to SO:
        // http://stackoverflow.com/a/2602258/841567
        ifs.seekg(0, std::ios::end);
        size_t filesize = ifs.tellg();
        response.assign(filesize, ' ');
        ifs.seekg(0, std::ios::beg);
        ifs.read(&response[0], filesize);
    }

    std::string header = format_header(filename, response);
    std::cout << "Response is: \"" << header + response << "\"" << std::endl;
    return header + response;
}

std::string SearchServer::format_header(const std::string& filename, const std::string& content)
{
    std::string header;
    header.append("HTTP/1.1 200 OK\r\n");
    header.append("Content-type: " + decide_content_type(filename) + "\r\n");
    header.append("Content-length: " + convert<std::string>(content.size()) + "\r\n");
    header.append("\r\n");
    return header;
}

std::string SearchServer::decide_content_type(const std::string& filename)
{
    std::string content_type;
    if (StringUtil::endswith(filename, ".html") or StringUtil::endswith(filename, ".htm"))
    {
        content_type = "text/html";
    }
    else if (StringUtil::endswith(filename, ".css"))
    {
        content_type = "text/css";
    }
    else if (StringUtil::endswith(filename, ".js"))
    {
        content_type = "application/javascript";
    }
    else
    {
        content_type = "text/plain";
    }
    return content_type;
}
