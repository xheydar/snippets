#include <iostream>
#include <yaml-cpp/yaml.h>

using namespace std;

int main( int argc, char** argv )
{
    YAML::Node config = YAML::LoadFile("../config.yml");

    for( YAML::const_iterator it=config.begin(); it != config.end() ; it++ )
    {
        cout << it -> first.as<string>() << "\t" << it -> second.IsMap() << endl;
        cout << it -> second.IsMap() << endl;
        cout << it -> second.IsSequence() << endl;
        cout << it -> second.IsScalar() << endl;

        if( it -> second.IsMap() )
        {
            YAML::Node node = it -> second;

            for( YAML::const_iterator j=node.begin() ; j != node.end() ; j++ )
            {
                cout << j -> first.as<string>() << "\t" << j -> second.as<string>() << endl;
            }
        }

        if( it -> second.IsSequence() )
        {
            cout << it -> second << endl;
            YAML::Node node = it -> second;
            for( YAML::const_iterator j=node.begin() ; j != node.end() ; j++ )
            {
                cout << j -> as<double>() << endl;
            }
        }  

    }

    return 0;
}