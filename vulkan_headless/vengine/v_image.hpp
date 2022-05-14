#pragma once

#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

#include "v_device.hpp"
#include "v_pipeline.hpp"

namespace vengine
{
	class v_image
	{
		private:
			v_device& device;
			std::unique_ptr<v_pipeline>& pipeline;

			uint32_t width;
			uint32_t height;
	
			VkFormat format;
			VkDeviceSize imageSize;
			VkImage textureImage;
			VkDeviceMemory textureImageMemory;
			VkImageView textureImageView;
			VkSampler textureSampler;

			void copyBuffer(VkBuffer srcBuffer, 
					        VkBuffer dstBuffer, 
							VkDeviceSize size);

			void transitionImageLayout(VkImage image, 
									   VkFormat format, 
									   VkImageLayout oldLayout, 
									   VkImageLayout newLayout);

			void copyBufferToImage(VkBuffer buffer, 
								   VkImage image, 
								   uint32_t width, 
								   uint32_t height);

			VkImageView createImageView(VkImage image, VkFormat format);

			void create_image( uint32_t width, uint32_t height, VkFormat format,
							   VkImageTiling tiling, VkImageUsageFlags usage,
							   VkMemoryPropertyFlags properties, VkImage& image,
							   VkDeviceMemory &imageMemory );
		public:
			v_image( v_device &d, std::unique_ptr<v_pipeline>& p, cv::Mat& src );
			~v_image();

			v_image( const v_image& ) = delete;
            void operator = ( const v_image& ) = delete;
            v_image( v_image&& ) = delete;
            v_image operator = ( v_image&& ) = delete;

			void create_texture_image_view();
			void create_texture_sampler();

			VkImageView& get_image_view() { return textureImageView; };
			VkSampler& get_sampler() { return textureSampler; };

			void bind();
	};
}
