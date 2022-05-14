#pragma once

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <opencv2/opencv.hpp>
#include <array>
#include <map>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

#include "base_tools.hpp"
#include "v_device.hpp"
#include "v_engine.hpp"
#include "v_pipeline.hpp"
#include "v_mesh.hpp"
#include "v_image.hpp"
#include "v_object.hpp"
#include "v_descriptor_pool.hpp"

#include "../base64/base64.hpp"

namespace vengine
{
    class module
    {
        private:
            frame_size fsize;
            v_device device;

            std::unique_ptr<v_engine> engine;
            std::map<std::string, std::unique_ptr<v_pipeline>> pipelines;

            std::unique_ptr<v_pipeline> pipeline;
            std::unique_ptr<v_pipeline> pipeline_copy;

            std::map<std::string,std::unique_ptr<v_mesh>> meshes;
            std::map<std::string,std::unique_ptr<v_texture>> textures;
            std::unique_ptr<v_descriptor_pool> descriptor_pool;

            std::vector<std::unique_ptr<v_object>> objects;

            int count = 0;

            void load_pipelines( nlohmann::json );
            void load_meshes( nlohmann::json );
            void load_textures( nlohmann::json );
            void load_objects( nlohmann::json ); 
        public:
            module( nlohmann::json cfg, frame_size );
            ~module();

            cv::Mat render();
    };
};
