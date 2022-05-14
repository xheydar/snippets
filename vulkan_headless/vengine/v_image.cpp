#include "v_image.hpp"

namespace vengine
{
	void v_image::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
		VkCommandBuffer commandBuffer = pipeline -> begin_single_time_commands(); 
		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		pipeline -> end_single_time_commands( commandBuffer );
	}

	void v_image::transitionImageLayout( VkImage image, 
										 VkFormat format, 
										 VkImageLayout oldLayout, 
										 VkImageLayout newLayout )
	{
		VkCommandBuffer commandBuffer = pipeline -> begin_single_time_commands(); 

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
				   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		} else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
				commandBuffer,
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
				);	
			
		pipeline -> end_single_time_commands(commandBuffer);
	}

	void v_image::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkCommandBuffer commandBuffer = pipeline -> begin_single_time_commands();
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = {0, 0, 0};
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
				commandBuffer,
				buffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&region
				);

		pipeline -> end_single_time_commands(commandBuffer);
	}

	VkImageView v_image::createImageView(VkImage image, VkFormat format)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		VK_CHECK_RESULT( vkCreateImageView(device.device(), &viewInfo, nullptr, &imageView) );
		return imageView;
	}

	void v_image::create_image( uint32_t width, uint32_t height, VkFormat format,
								VkImageTiling tiling, VkImageUsageFlags usage,
								VkMemoryPropertyFlags properties, VkImage& image,
								VkDeviceMemory &imageMemory )
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK_RESULT( vkCreateImage(device.device(), &imageInfo, nullptr, &image) );

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device.device(), image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = getMemoryTypeIndex( device.get_physical_device(), 
														memRequirements.memoryTypeBits, 
														properties);

		VK_CHECK_RESULT( vkAllocateMemory(device.device(), &allocInfo, nullptr, &imageMemory) );
		vkBindImageMemory(device.device(), image, imageMemory, 0);
	}


	v_image::v_image( v_device &d, std::unique_ptr<v_pipeline>& p, cv::Mat& src ) : device{d}, pipeline{p}
	{
		width = static_cast<uint32_t>(src.size().width);
		height = static_cast<uint32_t>(src.size().height);

		std::vector<uchar> pixels_buffer( src.total()*4 );	
		cv::Mat continuousRGBA(src.size(), CV_8UC4, pixels_buffer.data());
		cv::cvtColor( src, continuousRGBA, cv::COLOR_BGR2RGBA, 4);

		imageSize = width * height * 4;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		device.createBuffer( VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&stagingBuffer,
				&stagingBufferMemory,
				imageSize );

		void* data;
		vkMapMemory(device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels_buffer.data(), static_cast<size_t>(imageSize)); 
		vkUnmapMemory(device.device(), stagingBufferMemory);
		
		format = VK_FORMAT_R8G8B8A8_SRGB;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		create_image( width, height, format, tiling, usage, properties, textureImage, textureImageMemory );	

		transitionImageLayout(textureImage, 
							  VK_FORMAT_R8G8B8A8_SRGB, 
							  VK_IMAGE_LAYOUT_UNDEFINED, 
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
							  copyBufferToImage(stagingBuffer, textureImage, 
												static_cast<uint32_t>(width), 
												static_cast<uint32_t>(height) );

		transitionImageLayout(textureImage, 
							  VK_FORMAT_R8G8B8A8_SRGB, 
							  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
							  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);

	}

	v_image::~v_image()
	{
		vkDestroySampler(device.device(), textureSampler, nullptr);
		vkDestroyImageView(device.device(), textureImageView, nullptr);
		vkDestroyImage(device.device(), textureImage, nullptr);
		vkFreeMemory(device.device(), textureImageMemory, nullptr);
	}

	void v_image::create_texture_image_view()
	{
		textureImageView = createImageView(textureImage, format);
	}

	void v_image::create_texture_sampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device.get_physical_device(), &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VK_CHECK_RESULT( vkCreateSampler(device.device(), &samplerInfo, nullptr, &textureSampler) );

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;
/*
		auto descriptor_set = pipeline -> get_descriptor_set();

		VkWriteDescriptorSet descriptorWrites{};
		descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.dstSet = descriptor_set;
		descriptorWrites.dstBinding = 0;
		descriptorWrites.dstArrayElement = 0;
		descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites.descriptorCount = 1;
		descriptorWrites.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device.device(), 1, &descriptorWrites, 0, nullptr);

		auto pipelineLayout = pipeline -> get_pipeline_layout();
		auto commandBuffer = pipeline -> begin_single_time_commands();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptor_set, 0, nullptr);

		pipeline -> end_single_time_commands( commandBuffer );*/
	}

	void v_image::bind()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = &textureSampler;
	}
}
