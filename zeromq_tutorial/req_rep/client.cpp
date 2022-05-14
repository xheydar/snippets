#include <iostream>
#include <zmq.hpp>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::req );
    socket.connect("tcp://localhost:5555");

    for( int i=0 ; i<10 ; i++ )
    {
        zmq::message_t request(5);
        memcpy(request.data(), "Hello", 5);
        socket.send( request, zmq::send_flags::none );

        zmq::message_t reply;
        auto r = socket.recv( reply, zmq::recv_flags::none );
        cout << reply.size() << endl;
    }
    return 0;
}
