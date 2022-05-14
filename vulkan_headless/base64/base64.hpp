#ifndef BASE64_HPP
#define BASE64_HPP

#include <string>
#include <vector>

typedef unsigned char uchar;

namespace base64
{
    const std::string b = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";//=
    std::string encode(const std::vector<char> &in);
    std::vector<char> decode(const std::string &in);
}

#endif