#include <iostream>
#include <zmq.hpp>
#include <sstream>

using namespace std;

int main( int argc, char** argv )
{
    zmq::context_t context(1);
    zmq::socket_t subscriber( context, zmq::socket_type::sub );
    subscriber.connect("tcp://localhost:15555");

    // This is very important to make subscription work
    subscriber.set( zmq::sockopt::subscribe, "10001");

    long total_temp = 0;

    for( int i=0 ; i<100 ; i++ )
    {
        zmq::message_t update;
        int zipcode, temperature, relhumidity;

        auto rc = subscriber.recv(update, zmq::recv_flags::none);

        std::istringstream iss(static_cast<char*>(update.data()));
		iss >> zipcode >> temperature >> relhumidity ;

		total_temp += temperature;

        cout << total_temp << endl;
    }
    return 0;
}