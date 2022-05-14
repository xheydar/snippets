#include "irc.hpp"

using namespace twitch;
using namespace boost::asio::ip;

void irc::parse( std::vector<char> data, int len )
{
    std::vector<char> buffer;
    std::merge( intermediate.begin(), intermediate.end(),
                data.begin(), data.begin() + len,
                std::back_inserter( buffer ) );

    std::cout << buffer.size() << std::endl;

    std::string b = "\r\n";
    std::string message_tag = "PRIVMSG ";
    std::string splitter = ":";

    while( true )
    {
        auto it = std::search( buffer.begin(), buffer.end(), b.begin(), b.end() );
        
        if( it == buffer.end() )
        {
            if( buffer.size() != 0 )
            {
                intermediate = buffer;
            }
            else
            {
                intermediate.clear();
            }
            break;
        }

        std::string str( buffer.begin(), it );
        buffer = std::vector<char>( it+2, buffer.end() );

        
        // Now checking of what we got is a message
        it = std::search( str.begin(), str.end(), 
                          message_tag.begin(), message_tag.end() );

        if( it != str.end() )
        {
            message m;
            std::copy( str.begin()+1, it-1, std::back_inserter(m.username) );
            std::string tmp;
            std::copy( it + message_tag.size(), str.end(), std::back_inserter(tmp));

            it = std::search( tmp.begin(), str.end(),
                              splitter.begin(), splitter.end() );

            if( it != tmp.end() )
            {
                std::copy( tmp.begin(), it-1, std::back_inserter(m.channel) );
                std::copy( it+1, tmp.end(), std::back_inserter(m.content) );

                message_queue.push_back( m );
            }
        }
    }
}

irc::irc()
{}

irc::irc( YAML::Node cfg )
{
    address = cfg["address"].as<std::string>();
    port = cfg["port"].as<std::string>();
    oauth = cfg["oauth"].as<std::string>();
    channel_name = cfg["channel_name"].as<std::string>();
    nick_name = cfg["nick_name"].as<std::string>();
}

bool irc::connect()
{
    socket = std::shared_ptr<tcp::socket>( new tcp::socket(service) );
    tcp::resolver::query resolver_query( address, port );
    tcp::resolver resolver(service);

    tcp::resolver::iterator it = resolver.resolve( resolver_query, ec );
    auto endpoint = it -> endpoint();

    socket -> connect( endpoint );

    send("PASS " + oauth + "\r\n");
    send("NICK " + nick_name + "\r\n");
    send("USER " + channel_name + "\r\n");
    send("JOIN #" + channel_name + "\r\n");

    return true;
}

bool irc::send( std::string message )
{
    socket -> send( boost::asio::buffer(message) );

    return true;
}

void irc::listen()
{
    std::vector<char> buffer(1024);
    while( ec != boost::asio::error::eof )
    {
        auto buffer_size = socket -> read_some( boost::asio::buffer( buffer ), ec );
        parse( buffer, buffer_size );
 
    }
}

void irc::background_listen()
{
    auto t = std::thread( &irc::listen, this );
    t.detach();
}
