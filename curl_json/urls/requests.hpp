#ifndef REQUESTS_HPP
#define REQUESTS_HPP

#include <iostream>
#include <string>
#include <map>
#include <curl/curl.h>

using namespace std;

namespace urls
{  
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    struct response
    {
        string body;
        long status_code = -1;
    };

    class requests
    {
    private:
        CURL* curl;
        CURLcode res;

        string read_buffer;
        map<string,string> headers;

        string credentials = "";
    public:
        requests();
        ~requests();

        void add_header( string, string );
        void set_credentials( string, string );

        response get( string );
        response post( string, char*, long );
    };

};

#endif
