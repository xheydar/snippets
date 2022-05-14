#include <iostream>
#include <chrono>
#include <thread>
#include <functional>

void callback()
{
    std::cout << "This is the callback" << std::endl;
}

void timer( int duration, std::function<void()> callback_func )
{
    while( true )
    {
        std::this_thread::sleep_for( std::chrono::milliseconds(duration) );

        std::thread t0( callback_func );
        t0.detach();
    }
}

int main( int argc, char** argv )
{
    timer( 1000, callback );
    return 0;
}