#include <iostream>
#include <chrono>
#include <nlohmann/json.hpp>

int main( int argc, char** argv )
{
    std::chrono::time_point<std::chrono::steady_clock> t = std::chrono::steady_clock::now();
    auto t_micro_secs = std::chrono::time_point_cast<std::chrono::microseconds>(t);
    auto value = t_micro_secs.time_since_epoch();

    long long duration = value.count();

    std::cout << duration << std::endl;

    std::chrono::microseconds dur( duration );

    std::chrono::time_point<std::chrono::steady_clock> dt( dur ); 

    assert( dt == t_micro_secs );

    nlohmann::json json;
    json["timestamp"] = duration;

    std::cout << json.dump() << std::endl;

    return 0;
}