#include <iostream>
#include <zmq.hpp>

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::req );
    socket.connect("tcp://localhost:5555");

    std::string client_name( argv[1] );

    for( int i=0 ; i<100 ; i++ )
    {
        std::string message = client_name + std::to_string(i);
        zmq::message_t request( message.size() );
        memcpy( request.data(), message.c_str(), message.size() );

        socket.send(request, zmq::send_flags::none);

        zmq::message_t reply;
        auto r = socket.recv( reply, zmq::recv_flags::none );
    }

    return 0;
}