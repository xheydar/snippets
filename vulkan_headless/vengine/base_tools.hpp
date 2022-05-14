#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <functional>
#include <array>
#include <opencv2/opencv.hpp>

#include "VulkanTools.h"

namespace vengine
{
	const int MAX_FRAMES_IN_FLIGHT = 1;

	struct UniformBufferObject {
		glm::mat4 mvp;
		glm::mat4 normalMVP;
		glm::vec3 lightVector;
	};

	struct Vertex
	{
		glm::vec3 position{};
		glm::vec3 normal{};
		glm::vec3 color{};
		glm::vec2 uv{};

		std::size_t hash() const;
		static VkVertexInputBindingDescription get_binding_description();
		static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions(); 
	};

	struct frame_size
	{
		uint32_t width;
		uint32_t height;
		uint32_t channels;
	};

	uint32_t getMemoryTypeIndex( VkPhysicalDevice physicalDevice, 
                             uint32_t typeBits, 
                             VkMemoryPropertyFlags properties);
}

namespace vengine::tools
{
	std::pair<std::vector<Vertex>,std::vector<uint32_t>> load_obj_file( std::string );
	std::pair<std::vector<Vertex>,std::vector<uint32_t>> load_obj_memory( std::string &obj_str,
																		  std::string &mtl_str );

	std::pair<uchar*, frame_size> load_image_file( std::string );
	std::pair<uchar*, frame_size> load_image_memory( std::vector<char>& );
	std::vector<char> load_file( std::string );
}
