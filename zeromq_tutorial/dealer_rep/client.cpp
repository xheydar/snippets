#include <iostream>
#include <zmq.hpp>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::rep );
    socket.connect("tcp://localhost:5555");

    while( true )
    {
        zmq::message_t message;
        socket.recv( message, zmq::recv_flags::none );

        cout << message.size() << endl;

        zmq::message_t response(4);
        memcpy( response.data(), "Done", 4 );
    }

    return 0;
}