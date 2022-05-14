#include "module.hpp"

namespace vengine
{
    void module::load_pipelines( nlohmann::json cfg_json )
    {
        std::map<std::string,nlohmann::json> cfg = cfg_json;
        for( const auto &c : cfg )
        {
            auto shaderCodeVert = base64::decode(c.second["vert"]);
            auto shaderCodeFrag = base64::decode(c.second["frag"]);

            auto p = std::make_unique<v_pipeline>( device, engine, fsize,
                                                   shaderCodeVert,
                                                   shaderCodeFrag );
            pipelines[ c.first ] = std::move(p);

        }
    }

    void module::load_meshes( nlohmann::json cfg_json )
    {
        std::map<std::string,nlohmann::json> cfg = cfg_json;
        for( const auto &c : cfg )
        {
            std::string mesh_name = c.first;
            std::string type = c.second["type"];

            if( type == "obj" )
            { 
                std::string name = "";
                if( c.second.contains("name") )
                {
                    name = c.second["name"];
                }

                bool normalize = c.second["normalize"];

                auto mesh = std::make_unique<v_mesh>( device, normalize );

                auto obj_data = base64::decode( c.second["data_obj"] );
                auto mtl_data = base64::decode( c.second["data_mtl"] );

                //mesh -> from_obj_file( path_obj );
                mesh -> from_obj_memory( obj_data, mtl_data );
                mesh -> prepare_vertices_and_indices();
                meshes[mesh_name] = std::move( mesh );
            } 
            else if( type == "named" )
            {
                //std::cout << "Named" << std::endl;
            }
        }
    }

    void module::load_textures( nlohmann::json cfg_json )
    {
        std::map<std::string,nlohmann::json> cfg = cfg_json;
        for( const auto &c : cfg )
        {
            std::string name = c.first;

            auto texture = std::make_unique<v_texture>( device, engine );

            auto data = base64::decode( c.second["data"] );

            texture -> load_image_memory( data );
            textures[name] = std::move( texture );
        }
    }

    void module::load_objects( nlohmann::json cfg_json )
    {
        std::map<std::string,nlohmann::json> cfg = cfg_json;
        for( const auto &c : cfg )
        {
            std::string name = c.first;
            std::string mesh_name = "";
            if( c.second.contains("mesh") )
            {
                mesh_name = c.second["mesh"];
            }
            else
            {
                throw std::runtime_error("Mesh was not provided for object " + c.first);
            }

            int pool_idx = -1;
            if( c.second.contains("texture") )
            {
                std::string texture_name = c.second["texture"];
                pool_idx = textures[texture_name] -> pool_idx();
            }

            std::string pipeline_name;
            if( c.second.contains("pipeline") )
            {
                pipeline_name = c.second["pipeline"];
            }
            else
            {
                throw std::runtime_error("Pipeline was not provided for object " + c.first);
            }

            auto object = std::make_unique<v_object>( meshes[mesh_name],
                                                       pool_idx,
                                                       pipeline_name );

            object -> set_visibility( c.second["always_alive"],
                                      c.second["seconds_alive"],
                                      c.second["pretty_name"],
                                      c.second["token_value"] );
            
            objects.push_back(std::move(object));
        }
    }

    module::module( nlohmann::json cfg, frame_size s ) : fsize{s}
    { 
        engine = std::make_unique<v_engine>( device, fsize );

        load_pipelines( cfg["pipelines"] );
        load_meshes( cfg["meshes"] );
        load_textures( cfg["textures"] );
        
        descriptor_pool = std::make_unique<v_descriptor_pool>( device, engine, 
                                                               textures );

        load_objects( cfg["objects"] );
    }

    module::~module()
    {
    }

    cv::Mat module::render()
    {
        VkCommandBuffer commandBuffer;

        // Setting the clear screen values
        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };
 
        // Setting the viewport
        VkViewport viewport = {};
        viewport.height = (float)fsize.height;
        viewport.width = (float)fsize.width;
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;

        // Update dynamic scissor state
        VkRect2D scissor = {};
        scissor.extent.width = fsize.width;
        scissor.extent.height = fsize.height;

        auto pipelineLayout = engine -> get_pipelineLayout();

        commandBuffer = engine -> begin_renderpass_commands( clearValues );
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
 
        std::vector<glm::vec3> pos = {
            glm::vec3(-1.5f, 0.0f, -4.0f),
            glm::vec3( 0.0f, 0.0f, -2.0f),
            glm::vec3( 1.5f, 0.0f, -4.0f),
        };

        int idx = 0;
        for( const auto& object : objects )
        {
            if( ! object -> is_alive() )
            {
                idx++;
                continue;
            }
            auto pname = object -> pipeline_name();
            object -> bind( commandBuffer );
            pipelines[pname] -> bind( commandBuffer );
            
            descriptor_pool -> bind( commandBuffer, object -> pool_idx() );

            // Model Matrix
            glm::mat4 scale_matrix = glm::scale( glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
            glm::mat4 rotation_matrix = glm::rotate( glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0,0,1) ) *
                                        glm::rotate( glm::mat4(1.0f), glm::radians((360.f / 100) * count), glm::vec3(0,1,1) );
            glm::mat4 translate_matrix = glm::translate( glm::mat4(1.0f), pos[idx] );//glm::vec3(0.0f, 0.0f, 0.0f) );


            glm::mat4 ModelMatrix = translate_matrix * rotation_matrix * scale_matrix;

            glm::mat4 View = glm::lookAt(
                    glm::vec3(0,0,1), // Camera is at (4,3,3), in World Space
                    glm::vec3(0,0,0), // and looks at the origin
                    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                    );

            glm::mat4 Projection = glm::perspective( glm::radians(60.0f), 
                                                     (float) fsize.width / (float) fsize.height, 
                                                     0.1f, 50.0f);
            
            glm::mat4 mvpMatrix = Projection * View * ModelMatrix;
            glm::mat4 normalMVPMatrix = glm::transpose(glm::inverse( View*ModelMatrix ) );
            glm::vec3 lightVector( 0.4f, 0.4f, 4.0f );

            UniformBufferObject bufferObj = { mvpMatrix, normalMVPMatrix, lightVector };

            vkCmdPushConstants( commandBuffer, pipelineLayout, 
                                VK_SHADER_STAGE_VERTEX_BIT, 0, 
                                sizeof(bufferObj), &bufferObj);

            vkCmdDrawIndexed(commandBuffer, object -> count(), 1, 0, 0, 0);
            idx ++;
        }

        count ++;

        engine -> end_renderpass_commands( commandBuffer );

        vkDeviceWaitIdle(device.device());

        // This is very very slow
        auto overlay = engine -> copy_image_buffer();
        return overlay;
    }
}
