#include <iostream>
#include <zmq.hpp>

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::req );
    socket.connect("tcp://localhost:5555");

    for( int i=0 ; i<10 ; i++ )
    {
        zmq::message_t request(5);
        memcpy( request.data(), "Hello", 5 );

        socket.send( request, zmq::send_flags::none );

        zmq::message_t reply;
        auto rc = socket.recv( reply, zmq::recv_flags::none );

        std::string reply_str = std::string( reinterpret_cast<char*>( reply.data() ), reply.size() );

        std::cout << reply_str << std::endl;    
    }

    return 0;
}