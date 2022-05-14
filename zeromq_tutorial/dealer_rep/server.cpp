#include <iostream>
#include <zmq.hpp>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t dealer( context, zmq::socket_type::dealer );
    dealer.bind("tcp://*:5555");

    for( int i=0 ; i<10 ; i++ )
    {
        zmq::message_t message(5);
        memcpy( message.data(), "Hello", 5 );
        dealer.send( message, zmq::send_flags::none );


        zmq::message_t reply;
        dealer.recv( reply, zmq::recv_flags::none );

        cout << reply.size() << endl;
    }


    return 0;
}