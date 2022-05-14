#include <iostream>
#include <zmq.hpp>
#include <chrono>
#include <thread>

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::req );
    socket.connect("tcp://localhost:5560");

    int delay = atoi( argv[1] );
    std::cout << delay << std::endl;

    while( true )
    {
        zmq::message_t incoming;
        auto rc = socket.recv( incoming, zmq::recv_flags::none );

        std::string incoming_str = std::string( reinterpret_cast<char*>( incoming.data() ), incoming.size() );

        std::cout << incoming_str << std::endl;

        zmq::message_t outgoing(5);
        memcpy( outgoing.data(), "World", 5 );

        socket.send( outgoing, zmq::send_flags::none );

        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    return 0;
}