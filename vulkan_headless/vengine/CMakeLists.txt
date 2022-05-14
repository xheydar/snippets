cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

add_library(vengine VulkanTools.cpp base_tools.cpp v_device.cpp v_engine.cpp v_pipeline.cpp
                    v_descriptor_pool.cpp v_texture.cpp v_mesh.cpp v_object.cpp 
                    v_textured_model.cpp module.cpp)
