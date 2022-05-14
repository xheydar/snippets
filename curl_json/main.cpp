#include <iostream>
#include <string>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include "urls/requests.hpp"

using namespace std;
using json = nlohmann::json;

string fetch_token()
{
    json cred;
    cred["username"] = "heydar";
    cred["password"] = "12345";
    string cred_str = cred.dump();

    auto request = urls::requests();
    request.add_header("Content-Type", "application/json");
    auto res = request.post("http://10.0.1.26:8000/api/token-auth/", &cred_str[0], cred_str.size() );

    if( res.status_code == 200 )
    {
        json body = json::parse( res.body );
        string token = body["token"];
        return token;
    }
    else
    {
        return "";
    }
}

void verify_token( string token )
{
    json data;
    data["token"] = token;

    string data_str = data.dump();

    auto request = urls::requests();
    request.add_header("Content-Type", "application/json");
    auto res = request.post("http://10.0.1.26:8000/api/verify-token/", &data_str[0], data_str.size() );

    cout << res.status_code << endl;
    cout << res.body << endl;
}

string refresh_token( string token )
{
    json data;
    data["token"] = token;

    string data_str = data.dump();

    auto request = urls::requests();
    request.add_header("Content-Type", "application/json");
    auto res = request.post("http://10.0.1.26:8000/api/refresh-token/", &data_str[0], data_str.size() );

    if( res.status_code == 200 )
    {
        json body = json::parse( res.body );
        string token = body["token"];
        return token;
    }
    else
    {
        return "";
    }
}

void rest_post_with_token( string token, char* data, int data_size )
{
    auto request = urls::requests();
    request.add_header("Authorization", "JWT " + token);
    auto res = request.post("http://10.0.1.26:8000/api/post/", data, data_size );
    cout << res.status_code << endl;
}

int main( int argc, char** argv )
{    
    int data_size = 10;
    char* data = new char[data_size];
    for( int i=0 ; i<data_size ; i++ )
        data[i] = '1';

    /*
     *
     * HTTP GET WITH THE DJANGO SERVER WITHOUT AUTHENTICATION
     *
     */

    //auto request = urls::requests();
    //auto res = request.get("http://10.0.1.26:8000/normal/get/");
    //cout << res.body << endl;
    //cout << res.status_code << endl;
    
    /*
     *
     * HTTP POST WITH THE DJANGO SERVER WITHOUT AUTHENTICATION
     *
     */

    //auto request = urls::requests();
    //auto res = request.post("http://10.0.1.26:8000/normal/post/", data, data_size );
    //cout << res.status_code << endl;
    
    /*
     *
     * Fetching API token from the server
     *
     */

    string token = fetch_token();

    while( true )
    {    
        token = refresh_token( token );
        verify_token( token );
        sleep(1);
    }
    
    /*

    HTTP Post using token

    */

    //rest_post_with_token( token, data, data_size );

    delete[] data;
    return 0;
}
