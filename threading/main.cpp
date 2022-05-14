#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

class stoppable_thread
{
    private:
        bool should_stop = false;
        void thread_function();
    public:
        stoppable_thread();
        void run();
        void stop();

};

void stoppable_thread::thread_function()
{
    while( !should_stop )
    {
        cout << "Test" << "\t" << should_stop << endl;
        this_thread::sleep_for(chrono::seconds(1));
    } 
}

stoppable_thread::stoppable_thread()
{}

void stoppable_thread::run()
{
    should_stop = false;
    thread t( &stoppable_thread::thread_function, this );
    t.detach();
}

void stoppable_thread::stop()
{
    should_stop = true;
}

int main( int argc, char** argv )
{
    stoppable_thread s;
    s.run();

    while( true )
    {
        char x = getchar();
        cout << x << endl;
        if( x == 'x' )
        {
            s.stop();
            break;
        }
    }
    return 0;
}