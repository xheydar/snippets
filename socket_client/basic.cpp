#include <iostream>
#include <yaml-cpp/yaml.h>
#include <chrono>
#include <thread>
#include "twitch/irc.hpp"

int main( int argc, char** argv )
{
    auto twitch_cfg = YAML::LoadFile("../twitch.yml");
    twitch::irc irc( twitch_cfg["irc"] );

    irc.connect();
    irc.background_listen();

    while( true )
    {
        if( irc.message_queue.size() > 0 )
        {
            auto m = irc.message_queue[0];
            irc.message_queue.erase( irc.message_queue.begin() );
            std::cout << m.username << std::endl;
        }

        std::this_thread::sleep_for( std::chrono::milliseconds(10) );
    }

 
    return 0;
}

