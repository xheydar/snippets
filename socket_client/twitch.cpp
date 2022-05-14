#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>

#include "twitch/irc.hpp"

using namespace boost::asio::ip;

int main( int argc, char** argv )
{
    auto twitch_cfg = YAML::LoadFile("../twitch.yml");

    std::vector<char> buffer( 1024 );

    std::string address = twitch_cfg["server"].as<std::string>();
    std::string port = twitch_cfg["port"].as<std::string>();
    std::string oauth = twitch_cfg["oauth"].as<std::string>();

    boost::asio::io_service service;
    tcp::socket socket( service );

    boost::asio::ip::tcp::resolver::query resolver_query( address, port );
    boost::asio::ip::tcp::resolver resolver(service);

    boost::system::error_code ec;

    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(resolver_query, ec);

    auto endpoint = it -> endpoint();

    socket.connect( endpoint ); 

    std::string message;

    message = "PASS " + oauth + "\r\n";  
    socket.send( boost::asio::buffer( message ) );
    message = "NICK StreamPleat\r\n";
    socket.send( boost::asio::buffer( message ) );
    message = "USER xheydar\r\n";
    socket.send( boost::asio::buffer( message ) );
    message = "JOIN #xheydar\r\n";
    socket.send( boost::asio::buffer( message ) );

    while (ec != boost::asio::error::eof)
    {
        auto buffer_size = socket.read_some( boost::asio::buffer( buffer ), ec );

        std::string str( buffer.begin(), buffer.begin() + buffer_size );
        std::cout << str << std::endl;
    }

    return 0;
}
