#include <iostream>
#include <zmq.hpp>
#include <unistd.h>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::rep);
    socket.bind("tcp://*:5555");

    uint64_t count = 0;

    while( true )
    {
        zmq::message_t request;
        auto ret = socket.recv( request, zmq::recv_flags::none );

        cout << count << "\t" << request.size() << endl;

        zmq::message_t reply(5);
        memcpy(reply.data (), "World", 5);
        socket.send( reply, zmq::send_flags::none ); 

        count ++;
    }

    return 0;
}
