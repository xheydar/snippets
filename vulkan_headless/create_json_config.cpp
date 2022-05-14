#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <fstream>

#include "vengine/base_tools.hpp"

#include "base64/base64.hpp"

nlohmann::json create_json_config( YAML::Node yaml_cfg )
{
    nlohmann::json json_cfg;
    json_cfg["assets"] = nlohmann::json();
    json_cfg["assets"]["pipelines"] = nlohmann::json();

    std::map<std::string,YAML::Node> pipelines = yaml_cfg["pipelines"].as<std::map<std::string,YAML::Node>>();

    for( auto const &p : pipelines )
    {
        std::map<std::string,std::string> c = p.second.as<std::map<std::string,std::string>>();
        
        std::vector<char> shaderCodeVert = vengine::tools::load_file( c["vert"] );
        std::vector<char> shaderCodeFrag = vengine::tools::load_file( c["frag"] );

        json_cfg["assets"]["pipelines"][p.first]["vert"] = base64::encode( shaderCodeVert );
        json_cfg["assets"]["pipelines"][p.first]["frag"] = base64::encode( shaderCodeFrag );
    }

    std::map<std::string,YAML::Node> meshes = yaml_cfg["meshes"].as<std::map<std::string,YAML::Node>>();

    for( auto const &m : meshes )
    {
        std::map<std::string,std::string> c = m.second.as<std::map<std::string,std::string>>();
        
        json_cfg["assets"]["meshes"][m.first] = c;
        json_cfg["assets"]["meshes"][m.first]["normalize"] = c["normalize"] == "true";
        if( c["type"] == "obj" )
        {
            std::vector<char> data_obj = vengine::tools::load_file( c["path_obj"] );
            std::vector<char> data_mtl = vengine::tools::load_file( c["path_mtl"] );

            json_cfg["assets"]["meshes"][m.first]["data_obj"] = base64::encode( data_obj );
            json_cfg["assets"]["meshes"][m.first]["data_mtl"] = base64::encode( data_mtl );
        }

    }
 
    std::map<std::string,YAML::Node> textures = yaml_cfg["textures"].as<std::map<std::string,YAML::Node>>();

    for( auto const &t : textures )
    {
        std::map<std::string,std::string> c = t.second.as<std::map<std::string,std::string>>();

        std::vector<char> data = vengine::tools::load_file( c["path"] );
        //json_cfg["assets"]["textures"][t.first] = c;
        json_cfg["assets"]["textures"][t.first]["data"] = base64::encode( data );
    }

    std::map<std::string,YAML::Node> objects = yaml_cfg["objects"].as<std::map<std::string,YAML::Node>>();

    for( auto const &o : objects )
    {
        std::map<std::string,std::string> c = o.second.as<std::map<std::string,std::string>>();
        json_cfg["assets"]["objects"][o.first] = c;
        json_cfg["assets"]["objects"][o.first]["always_alive"] = c["always_alive"] == "true";
        json_cfg["assets"]["objects"][o.first]["seconds_alive"] = atoi(c["seconds_alive"].c_str());
        json_cfg["assets"]["objects"][o.first]["pretty_name"] = c["pretty_name"];
        json_cfg["assets"]["objects"][o.first]["token_value"] = atoi(c["token_value"].c_str());
    }

    return json_cfg;
}

int main( int argc, char** argv )
{
    YAML::Node config = YAML::LoadFile("../config.yml");
    nlohmann::json config_json = create_json_config( config );

    std::ofstream outfile("../config.json");
    outfile << config_json;

    return 0;
}
