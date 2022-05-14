#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#include "v_device.hpp"
#include "v_engine.hpp"
#include "base_tools.hpp"

namespace vengine
{
	class v_texture
	{
		private:
			v_device& device;
			std::unique_ptr<v_engine>& engine;

			VkImage textureImage;
			VkDeviceMemory textureImageMemory;
			VkImageView textureImageView;
			VkSampler textureSampler;

			int pool_idx_ = -1;

			void transition_image_layout(VkImage image, VkFormat format, 
										 VkImageLayout oldLayout, 
										 VkImageLayout newLayout);

			void copy_buffer_to_image(VkBuffer buffer, VkImage image, 
									  uint32_t width, uint32_t height);
		public:
			v_texture( v_device& d,
					   std::unique_ptr<v_engine>& engine );
			~v_texture();

			void load_image_file( std::string filename );
			void load_image_memory( std::vector<char> &data );
			void load( uchar* pixels, frame_size tex_size );

			VkImage& get_image() { return textureImage; };
			VkDeviceMemory& get_image_memory() { return textureImageMemory; };
			VkImageView& get_image_view() { return textureImageView; };
			VkSampler& get_texture_sampler() { return textureSampler; };

			int& pool_idx() { return pool_idx_; };
	};
}
