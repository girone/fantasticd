#ifndef SEARCHSERVER_H_
#define SEARCHSERVER_H_

#include <boost/asio.hpp>
#include "./InvertedIndex.h"

using boost::asio::ip::tcp;

class SearchServer
{
public:
    SearchServer(const unsigned int port, const InvertedIndex& ii);
    void run();

    static std::string format_response_header(const std::string& content_type, const std::string& content);
    static std::string decide_mime_type(const std::string& filename);

private:
    const unsigned int port_;
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;

    InvertedIndex ii_;

    std::string compute_HTTP_response(const std::string& request) const;
    std::string compute_file_response(const std::string& filename) const;
    std::string compute_autocomplete_response(const std::string& URL) const;
    std::string compute_search_response(const std::string& URL) const;
};


#endif  // SEARCHSERVER_H_
