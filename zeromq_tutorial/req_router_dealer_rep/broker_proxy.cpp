#include <iostream>
#include <zmq.hpp>
#include <chrono>

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t router( context, zmq::socket_type::router );
    zmq::socket_t dealer( context, zmq::socket_type::dealer );

    router.bind("tcp://*:5555");
    dealer.bind("tcp://*:5560");

    zmq::proxy( router, dealer );

    return 0;
}