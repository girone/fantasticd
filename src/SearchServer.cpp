/*
 * SearchServer.cpp
 *
 *  Created on: 11.03.2015
 *      Author: jonas
 */

#include "./SearchServer.h"
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include "./InvertedIndex.h"
#include "./StringUtil.h"

using boost::asio::ip::tcp;

// Turns exception handling in the run loop off. Useful for debugging.
// #define NO_EXCEPTIONS

SearchServer::SearchServer(const unsigned int port) :
    port_(port),
    io_service_(),
    acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_))
{
    ii_.create_from_ICD_HTML("icd/html/");
}

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
    std::string URL = request.substr(5, end_of_URL - 5);

    std::string content;
    std::string header;

    if (StringUtil::startswith(URL, "?ac="))
    {
        if (URL.size() < 4 + 3)
        {
            content = "[]";
        }
        else
        {
            content = compute_autocomplete_response(URL.substr(4));
        }
        header = format_response_header("js", content);
    }
    else if (StringUtil::startswith(URL, "?q="))
    {
        if (URL.size() == 3)
        {
            content = "[]";
        }
        else
        {
            content = compute_search_response(URL.substr(3));
        }
        header = format_response_header("json", content);
    }
    else
    {
        content = compute_file_response(URL);
        header = format_response_header(URL, content);
    }

    std::cout << "Response is: \"" << header + content << "\"" << std::endl;
    return header + content;
}

std::string SearchServer::format_response_header(const std::string& content_type, const std::string& content)
{
    std::string header;
    header.append("HTTP/1.1 200 OK\r\n");
    header.append("Content-type: " + decide_mime_type(content_type) + "\r\n");
    header.append("Content-length: " + convert<std::string>(content.size()) + "\r\n");
    header.append("\r\n");
    return header;
}

std::string SearchServer::decide_mime_type(const std::string& content_type)
{
    std::string mime_type;
    if (content_type == "json")
    {
        mime_type = "application/json";
    }
    else if (StringUtil::endswith(content_type, "html") or StringUtil::endswith(content_type, "htm"))
    {
        mime_type = "text/html";
    }
    else if (StringUtil::endswith(content_type, "css"))
    {
        mime_type = "text/css";
    }
    else if (StringUtil::endswith(content_type, "js"))
    {
        mime_type = "application/javascript";
    }
    else
    {
        mime_type = "text/plain";
    }
    return mime_type;
}

std::string SearchServer::compute_file_response(const std::string& filename) const
{
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
        std::string content_type = decide_mime_type(filename);

        // Read in the whole file.
        // More efficient than line by line reading according to SO:
        // http://stackoverflow.com/a/2602258/841567
        ifs.seekg(0, std::ios::end);
        size_t filesize = ifs.tellg();
        response.assign(filesize, ' ');
        ifs.seekg(0, std::ios::beg);
        ifs.read(&response[0], filesize);
    }
    return response;
}

std::string SearchServer::compute_autocomplete_response(const std::string& input) const
{
    std::string keyword = StringUtil::tolower(input);
    unsigned int delta = keyword.size() / 4;
    std::vector<std::string> suggestions = ii_.suggest(keyword, &delta);

    std::string json;
    if (suggestions.size() > 0)
    {
        json = "[\"" + StringUtil::join("\", \"", suggestions) + "\"]";
    }
    else
    {
        json = "[]";
    }
    // TODO(Jonas): Make this (and any other external request-based operation) work for UTF8.
    return json;
}

std::string SearchServer::compute_search_response(const std::string& input) const
{
    std::vector<std::string> keywords = StringUtil::split_string(StringUtil::tolower(input), ' ');
    std::vector<std::string> titles_and_URLs = ii_.format_search_result_title_and_URL_JSON(
            ii_.search(keywords)
    );
    std::string json = "[" + StringUtil::join(",", titles_and_URLs) + "]";
    return json;
}
