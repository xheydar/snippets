#ifndef TWITCH_IRC_HPP
#define TWITCH_IRC_HPP

#include <iostream>
#include <string>
#include <thread>
#include <yaml-cpp/yaml.h>
#include <boost/asio.hpp>

namespace twitch
{
    struct message
    {
        std::string username;
        std::string channel;
        std::string content;
    };

    class irc
    {
        private:
            std::string address;
            std::string port;
            std::string oauth;
            std::string nick_name;
            std::string channel_name;

            boost::system::error_code ec;
            boost::asio::io_service service;
            std::shared_ptr<boost::asio::ip::tcp::socket> socket;
            std::vector<char> intermediate;

            void parse( std::vector<char>, int );
        public:
            std::vector<message> message_queue;
            irc();
            irc( YAML::Node );

            bool connect();
            bool send( std::string );

            void listen();
            void background_listen();
    };
}

#endif
