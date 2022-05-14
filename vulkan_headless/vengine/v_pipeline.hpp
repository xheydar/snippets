#pragma once

#include <iostream>
#include <array>
#include <opencv2/opencv.hpp>
#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "base_tools.hpp"
#include "v_device.hpp"
#include "v_engine.hpp"

namespace vengine
{

	class v_pipeline
	{
		private :
			frame_size fsize;
			v_device& device;
			std::unique_ptr<v_engine>& engine;

			// Creating Buffer Attachments
			VkFormat colorFormat;
			VkFormat depthFormat;

			VkFramebuffer framebuffer;
			FrameBufferAttachment colorAttachment, depthAttachment;
			VkRenderPass renderPass;

			// Prepare Vertex and Index Buffers
			std::vector<VkShaderModule> shaderModules;
			VkBuffer vertexBuffer, indexBuffer;
			VkDeviceMemory vertexMemory, indexMemory;
			VkDescriptorSetLayout descriptorSetLayout;

			VkPipelineLayout pipelineLayout;
			VkPipeline pipeline;
			VkPipelineCache pipelineCache;

			VkShaderModule load_shader( std::vector<char> &shaderCode );

			void create_graphics_pipeline( std::vector<char> &shaderCodeVert,
										   std::vector<char> &shaderCodeFrag );
		public :
			v_pipeline( v_device& d, std::unique_ptr<v_engine>& e,  frame_size s,
						std::vector<char> &shaderCodeVert, std::vector<char> &shaderCodeFrag );
			~v_pipeline();

			/*v_pipeline( const v_pipeline& ) = delete;
            void operator = ( const v_pipeline& ) = delete;
            v_pipeline( v_pipeline&& ) = delete;
            v_pipeline operator = ( v_pipeline&& ) = delete;*/

			void bind( VkCommandBuffer &commandBuffer );

			//VkFramebuffer& get_frame_buffer() { return framebuffer; };
			//VkRenderPass& get_rende_pass() { return renderPass; };
			//VkPipelineLayout& get_pipeline_layout() { return pipelineLayout; };
			//FrameBufferAttachment get_color_attachment() { return colorAttachment; };
	};
}
