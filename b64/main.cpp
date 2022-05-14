#include <iostream>
#include <string>
#include <vector>
#include "base64/base64.hpp"

int main( int argc, char** argv )
{
    std::string input = "SDFSAF sfalsjkhdfkjsah fksbfas ffalse hello world";
    std::vector<char> input_data( input.begin(), input.end() );
    std::string input_b64 = base64::encode( input_data );
    std::cout << input_b64 << std::endl;
    std::vector<char> decoded = base64::decode( input_b64 );
    std::string decoded_str = std::string( decoded.begin(), decoded.end() );
    std::cout << decoded_str << std::endl;

    return 0;
}