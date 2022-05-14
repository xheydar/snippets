#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>
#include "base_tools.hpp"
#include "v_device.hpp"

namespace vengine
{
	class v_engine
	{
		private:
			v_device& device;
			frame_size fsize;

			// Creating Buffer Attachments
			VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
			VkFormat depthFormat;
			VkFramebuffer framebuffer;
			FrameBufferAttachment colorAttachment, depthAttachment;
			VkRenderPass renderPass;

			VkDescriptorSetLayout descriptorSetLayout;
			VkPipelineLayout pipelineLayout;


			void create_buffer_attachment();
			void create_render_pass();
			void create_descriptor_set_layout();
			void create_pipeline_layout();
		public:
			v_engine( v_device&, frame_size );
			~v_engine();

			VkFormat& get_colorFormat() {return colorFormat;};
			VkFormat& get_depthFormat() {return depthFormat;};
			VkFramebuffer& get_framebuffer() { return framebuffer; };
			FrameBufferAttachment& get_colorAttachment() { return colorAttachment;};
			FrameBufferAttachment& get_depthAttachment() { return depthAttachment;};

			VkDescriptorSetLayout& get_descriptSetLayout() { return descriptorSetLayout; };
			VkPipelineLayout& get_pipelineLayout() { return pipelineLayout; };

			VkRenderPass& get_rende_pass() { return renderPass; };

			VkCommandBuffer begin_single_time_commands();
			void end_single_time_commands( VkCommandBuffer &commandBuffer );

			VkCommandBuffer begin_renderpass_commands( std::array<VkClearValue, 2> clearValues );
			void end_renderpass_commands( VkCommandBuffer& commandBuffer );

			cv::Mat copy_image_buffer();
	};
}
