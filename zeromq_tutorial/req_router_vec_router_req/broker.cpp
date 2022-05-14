#include <iostream>
#include <zmq.hpp>
#include <queue>
#include <thread>
#include <chrono>

std::queue<std::shared_ptr<zmq::message_t>> internal_queue;

void receiver( zmq::context_t* context )
{
    //zmq::context_t context(1);
    zmq::socket_t socket( *context, zmq::socket_type::router );
    socket.bind("tcp://*:5555");

    std::cout << "Receiver setup." << "\n";

    while( true )
    {
        zmq::message_t client_identity;
        zmq::message_t client_dummy;
        zmq::message_t client_data;

        auto r = socket.recv(client_identity, zmq::recv_flags::none);
        r = socket.recv(client_dummy, zmq::recv_flags::none);
        r = socket.recv(client_data, zmq::recv_flags::none);

        //std::cout << client_data.size() << "\n";

        auto buffer = std::make_shared<zmq::message_t>( client_data.size() );
        memcpy( buffer -> data(), client_data.data(), client_data.size() );
        internal_queue.push( buffer );
        // Should make a copy of the data and send it into an internal queue.

        //std::cout << internal_queue.size() << "\n";

        zmq::message_t empty(0);
        socket.send( client_identity, zmq::send_flags::sndmore );
        socket.send( empty, zmq::send_flags::sndmore ); 
        socket.send( empty, zmq::send_flags::none ); 
    }
} 

void distributer( zmq::context_t* context )
{
    zmq::socket_t socket( *context, zmq::socket_type::router );
    socket.bind("tcp://*:5560");

    std::cout << "Distributer setup." << "\n";

    while( true )
    {
        if( internal_queue.size() > 0 )
        {
            // This is the message structure coming from REQ
            zmq::message_t worker_identity;
            zmq::message_t dummy;
            zmq::message_t message;
            auto rc = socket.recv( worker_identity, zmq::recv_flags::none );
            rc = socket.recv( dummy, zmq::recv_flags::none );
            rc = socket.recv( message, zmq::recv_flags::none );

            //std::cout << std::string( reinterpret_cast<char*>(message.data() ), message.size() )  << "\n";
        
            // This is the message structure replying to the right REQ
            zmq::message_t empty(0);
            socket.send( worker_identity, zmq::send_flags::sndmore );
            socket.send( empty, zmq::send_flags::sndmore );

            auto data = internal_queue.front();

            std::cout << internal_queue.size() << "\n";

            //std::cout << count << std::endl;
            //zmq::message_t out( count_str.size() );
            //memcpy( out.data(), count_str.c_str(), count_str.size() );

            socket.send( *data, zmq::send_flags::none );
            internal_queue.pop();
        }
        std::this_thread::sleep_for( std::chrono::milliseconds(1) ); 
    }
}

int main( int argc, char** argv )
{
    zmq::context_t context(1); 

    auto receiver_thread = std::thread( receiver, &context );
    receiver_thread.detach();

    auto distributer_thread = std::thread( distributer, &context );
    distributer_thread.detach();

    while( true )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(10) );
    }
    
    //receiver( &context );
    return 0;
}