#include <iostream>
#include <zmq.hpp>
#include <queue>
#include <thread>
#include <chrono>

int main( int argc, char** argv )
{
    zmq::context_t context(1); 

    zmq::socket_t receiver( context, zmq::socket_type::router );
    receiver.bind("tcp://*:5555");
    zmq::socket_t distributer( context, zmq::socket_type::router );
    distributer.bind("tcp://*:5560");

    while( true )
    {
        zmq::message_t client_identity;
        zmq::message_t client_dummy;
        zmq::message_t client_data;

        auto r = receiver.recv(client_identity, zmq::recv_flags::none);
        r = receiver.recv(client_dummy, zmq::recv_flags::none);
        r = receiver.recv(client_data, zmq::recv_flags::none);

        zmq::message_t empty(0);
        receiver.send( client_identity, zmq::send_flags::sndmore );
        receiver.send( empty, zmq::send_flags::sndmore ); 
        receiver.send( empty, zmq::send_flags::none ); 

        zmq::message_t worker_identity;
        zmq::message_t dummy;
        zmq::message_t message;
        auto rc = distributer.recv( worker_identity, zmq::recv_flags::none );
        rc = distributer.recv( dummy, zmq::recv_flags::none );
        rc = distributer.recv( message, zmq::recv_flags::none );

        // This is the message structure replying to the right REQ
        distributer.send( worker_identity, zmq::send_flags::sndmore );
        distributer.send( empty, zmq::send_flags::sndmore );

        distributer.send( client_data, zmq::send_flags::none );
    }

    return 0;
}
