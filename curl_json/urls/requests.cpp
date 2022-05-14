#include "requests.hpp"

urls::requests::requests()
{
    curl = curl_easy_init();
}

urls::requests::~requests()
{
    curl_easy_cleanup(curl);
}

void urls::requests::add_header( string h, string v )
{
    headers[h] = v;
}

void urls::requests::set_credentials( string username, string password )
{
    credentials = username + ":" + password;
}

urls::response urls::requests::get( string url )
{
    urls::response r;

    if( curl )
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, urls::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

        if( credentials.size() > 0 )
        {
            curl_easy_setopt(curl, CURLOPT_USERPWD, credentials.c_str());
        }

        // There are headers to add to the URL
        if( headers.size() > 0 )
        {
            struct curl_slist *headerlist = NULL;
            for( auto const& x : headers )
            {
                string header_string = x.first + ": " + x.second;
                headerlist = curl_slist_append(headerlist, header_string.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        }
    
        res = curl_easy_perform(curl);

        r.status_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &r.status_code);
    }

     return r;
}

urls::response urls::requests::post( string url, char* data, long data_size )
{
    urls::response r;

    if( curl )
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str() );
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, urls::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data_size);

        if( credentials.size() > 0 )
        {
            curl_easy_setopt(curl, CURLOPT_USERPWD, credentials.c_str());
        }

        // There are headers to add to the URL
        if( headers.size() > 0 )
        {
            struct curl_slist *headerlist = NULL;
            for( auto const& x : headers )
            {
                string header_string = x.first + ": " + x.second;
                headerlist = curl_slist_append(headerlist, header_string.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
        }

        res = curl_easy_perform(curl);

        r.status_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &r.status_code);
    }

    return r;
}
