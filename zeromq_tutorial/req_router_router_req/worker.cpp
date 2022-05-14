#include <iostream>
#include <zmq.hpp>
#include <chrono>
#include <thread>

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t socket( context, zmq::socket_type::req );

    socket.connect("tcp://localhost:5560");

    int count = 0;
    int delay = atoi(argv[1]);

    while( true )
    {
        zmq::message_t message(6);
        memcpy(message.data(), "READY", 6);
        socket.send( message, zmq::send_flags::none );

        zmq::message_t workload;
        auto rc = socket.recv( workload, zmq::recv_flags::none );
        //std::cout << ++count << "\t" << workload.size() << "\n";

        //std::cout << std::string(reinterpret_cast<char*>(workload.data()), workload.size()) << "\n";
		std::cout << workload.size() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    return 0;
}
