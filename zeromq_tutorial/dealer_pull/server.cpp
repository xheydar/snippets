#include <iostream>
#include <zmq.hpp>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::dealer );
    socket.bind("tcp://*:5555");

    int count = 0;

    while( true )
    {
        string count_str = to_string(count);
        zmq::message_t message( count_str.size() );
        memcpy( message.data(), count_str.c_str(), count_str.size() );

        cout << "Here" << endl;

        socket.send( message, zmq::send_flags::none );

        cout << count << endl;

        count = count + 1;
    }

    return 0;
}