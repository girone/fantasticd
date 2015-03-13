#ifndef SEARCHSERVER_H_
#define SEARCHSERVER_H_

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class SearchServer
{
public:
    SearchServer(const unsigned int port) :
        port_(port),
        io_service_(),
        acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_))
    { }
    void run();

private:
    const unsigned int port_;
    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::string compute_HTTP_response(const std::string& request) const;
};

#endif  // SEARCHSERVER_H_

