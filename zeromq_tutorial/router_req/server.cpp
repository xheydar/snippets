#include <iostream>
#include <zmq.hpp>

int main( int argc, char** argv )
{
    zmq::context_t context(1);

    zmq::socket_t router( context, zmq::socket_type::router );
    router.bind("tcp://*:5555");

    int count = 0;

    while( true )
    {
        // This is the message structure coming from REQ
        zmq::message_t worker_identity;
        zmq::message_t dummy;
        zmq::message_t message;
        auto rc = router.recv( worker_identity, zmq::recv_flags::none );

        std::cout << worker_identity.size() << std::endl;
        rc = router.recv( dummy, zmq::recv_flags::none );
        rc = router.recv( message, zmq::recv_flags::none );

        std::cout << std::string( reinterpret_cast<char*>(message.data() ), message.size() )  << "\n";
        
        // This is the message structure replying to the right REQ
        zmq::message_t empty(0);
        router.send( worker_identity, zmq::send_flags::sndmore );
        router.send( empty, zmq::send_flags::sndmore );
        
        count = count + 1;
        std::string count_str = std::to_string( count );
        std::cout << count << std::endl;
        zmq::message_t out( count_str.size() );
        memcpy( out.data(), count_str.c_str(), count_str.size() );

        router.send( out, zmq::send_flags::none );
    }

    return 0;
}