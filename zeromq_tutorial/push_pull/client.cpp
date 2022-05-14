#include <iostream>
#include <zmq.hpp>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::pull );

    socket.connect("tcp://localhost:5555");

    for( int ii=0 ; ii<10 ; ii++ )
    {
        for( int i=0 ; i<100 ; i++ )
        {
            zmq::message_t message;
            auto rc = socket.recv( message, zmq::recv_flags::none );

            cout << string( reinterpret_cast<char*>(message.data()), message.size() ) << endl;
        }
        getchar();
    }

    return 0;
}