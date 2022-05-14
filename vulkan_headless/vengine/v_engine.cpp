#include "v_engine.hpp"

namespace vengine
{
	void v_engine::create_buffer_attachment()
	{
		// Color attachment
		VkImageCreateInfo image = vks::initializers::imageCreateInfo();
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = colorFormat;
		image.extent.width = fsize.width;
		image.extent.height = fsize.height;
		image.extent.depth = 1;
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		auto physicalDevice = device.get_physical_device();
		vks::tools::getSupportedDepthFormat(physicalDevice, &depthFormat);

		VK_CHECK_RESULT(vkCreateImage(device.device(), &image, nullptr, &colorAttachment.image));
		vkGetImageMemoryRequirements(device.device(), colorAttachment.image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = getMemoryTypeIndex( physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAlloc, nullptr, &colorAttachment.memory));
		VK_CHECK_RESULT(vkBindImageMemory(device.device(), colorAttachment.image, colorAttachment.memory, 0));

		VkImageViewCreateInfo colorImageView = vks::initializers::imageViewCreateInfo();
		colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorImageView.format = colorFormat;
		colorImageView.subresourceRange = {};
		colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorImageView.subresourceRange.baseMipLevel = 0;
		colorImageView.subresourceRange.levelCount = 1;
		colorImageView.subresourceRange.baseArrayLayer = 0;
		colorImageView.subresourceRange.layerCount = 1;
		colorImageView.image = colorAttachment.image;
		VK_CHECK_RESULT(vkCreateImageView(device.device(), &colorImageView, nullptr, &colorAttachment.view));

		// Depth stencil attachment
		image.format = depthFormat;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VK_CHECK_RESULT(vkCreateImage(device.device(), &image, nullptr, &depthAttachment.image));
		vkGetImageMemoryRequirements(device.device(), depthAttachment.image, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		memAlloc.memoryTypeIndex = getMemoryTypeIndex( physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAlloc, nullptr, &depthAttachment.memory));
		VK_CHECK_RESULT(vkBindImageMemory(device.device(), depthAttachment.image, depthAttachment.memory, 0));

		VkImageViewCreateInfo depthStencilView = vks::initializers::imageViewCreateInfo();
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = depthFormat;
		depthStencilView.flags = 0;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;
		depthStencilView.image = depthAttachment.image;
		VK_CHECK_RESULT(vkCreateImageView(device.device(), &depthStencilView, nullptr, &depthAttachment.view));
	}

	void v_engine::create_render_pass()
	{
		VkFormat depthFormat;
		vks::tools::getSupportedDepthFormat(device.get_physical_device(), &depthFormat);

		std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
		// Color attachment
		attchmentDescriptions[0].format = colorFormat;
		attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		// Depth attachment
		attchmentDescriptions[1].format = depthFormat;
		attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
		renderPassInfo.pAttachments = attchmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();
		VK_CHECK_RESULT(vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass));

		VkImageView attachments[2];
		attachments[0] = colorAttachment.view;
		attachments[1] = depthAttachment.view;

		VkFramebufferCreateInfo framebufferCreateInfo = vks::initializers::framebufferCreateInfo();
		framebufferCreateInfo.renderPass = renderPass;
		framebufferCreateInfo.attachmentCount = 2;
		framebufferCreateInfo.pAttachments = attachments;
		framebufferCreateInfo.width = fsize.width;
		framebufferCreateInfo.height = fsize.height;
		framebufferCreateInfo.layers = 1;
		VK_CHECK_RESULT(vkCreateFramebuffer(device.device(), &framebufferCreateInfo, nullptr, &framebuffer));
	}

	void v_engine::create_descriptor_set_layout()
	{
		/*VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
		*/

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			//uboLayoutBinding,
			samplerLayoutBinding
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo{};
		descriptorLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorLayoutCreateInfo.pBindings = setLayoutBindings.data();

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.device(), &descriptorLayoutCreateInfo, 
													nullptr, &descriptorSetLayout));
	}

	void v_engine::create_pipeline_layout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(nullptr, 0);

		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// The pipeline layout is based on the descriptor set layout we created above
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;

		// UniformBufferObject via push constant block
		VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(UniformBufferObject), 0);
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

		VK_CHECK_RESULT(vkCreatePipelineLayout(device.device(), &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));
	}

	v_engine::v_engine( v_device& d, frame_size s ): device{d}, fsize{s}
	{
		create_buffer_attachment();
		create_render_pass();
		create_descriptor_set_layout();
		create_pipeline_layout();
	}

	v_engine::~v_engine()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);
		vkDestroyImageView(device.device(), colorAttachment.view, nullptr);
		vkDestroyImage(device.device(), colorAttachment.image, nullptr);
		vkFreeMemory(device.device(), colorAttachment.memory, nullptr);
		vkDestroyImageView(device.device(), depthAttachment.view, nullptr);
		vkDestroyImage(device.device(), depthAttachment.image, nullptr);
		vkFreeMemory(device.device(), depthAttachment.memory, nullptr);
		vkDestroyRenderPass(device.device(), renderPass, nullptr);
		vkDestroyFramebuffer(device.device(), framebuffer, nullptr);
	}


	VkCommandBuffer v_engine::begin_single_time_commands()
	{
		auto commandPool = device.get_command_pool();
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.device(), &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; 
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo); 
        return commandBuffer;
	}

	void v_engine::end_single_time_commands( VkCommandBuffer& commandBuffer )
	{
		VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
		device.submit_work( commandBuffer );
	
		auto commandPool = device.get_command_pool();
		vkFreeCommandBuffers(device.device(), commandPool, 1, &commandBuffer);
	}

	VkCommandBuffer v_engine::begin_renderpass_commands( std::array<VkClearValue, 2> clearValues )
	{
		auto commandPool = device.get_command_pool();
		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vks::initializers::commandBufferAllocateInfo(commandPool, 
					VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device.device(), 
					&cmdBufAllocateInfo, 
					&commandBuffer));

		VkCommandBufferBeginInfo cmdBufInfo =
			vks::initializers::commandBufferBeginInfo();

		VK_CHECK_RESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufInfo));

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderArea.extent.width = fsize.width;
		renderPassBeginInfo.renderArea.extent.height = fsize.height;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.renderPass = get_rende_pass();
		renderPassBeginInfo.framebuffer = get_framebuffer();

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		return commandBuffer;
	}

	void v_engine::end_renderpass_commands( VkCommandBuffer &commandBuffer )
	{
		vkCmdEndRenderPass(commandBuffer);
        VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));
        device.submit_work( commandBuffer ); 
		
		auto commandPool = device.get_command_pool();
		vkFreeCommandBuffers(device.device(), commandPool, 1, &commandBuffer);
	}

	cv::Mat v_engine::copy_image_buffer()
	{
		char* imagedata;
		// Create the linear tiled destination image to copy to and to read the memory from
		VkImageCreateInfo imgCreateInfo(vks::initializers::imageCreateInfo());
		imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imgCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imgCreateInfo.extent.width = fsize.width;
		imgCreateInfo.extent.height = fsize.height;
		imgCreateInfo.extent.depth = 1;
		imgCreateInfo.arrayLayers = 1;
		imgCreateInfo.mipLevels = 1;
		imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		imgCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;


		auto physicalDevice = device.get_physical_device();
		auto commandPool = device.get_command_pool();
		//auto colorAttachment = get_color_attachment();


		// Create the image
		VkImage dstImage;
		VK_CHECK_RESULT(vkCreateImage(device.device(), &imgCreateInfo, nullptr, &dstImage));
		// Create memory to back up the image
		VkMemoryRequirements memRequirements;
		VkMemoryAllocateInfo memAllocInfo(vks::initializers::memoryAllocateInfo());
		VkDeviceMemory dstImageMemory;
		vkGetImageMemoryRequirements(device.device(), dstImage, &memRequirements);
		memAllocInfo.allocationSize = memRequirements.size;
		// Memory must be host visible to copy from
		memAllocInfo.memoryTypeIndex = getMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, 
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &dstImageMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device.device(), dstImage, dstImageMemory, 0));

		// Do the actual blit from the offscreen image to our host visible destination image
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VkCommandBuffer copyCmd;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device.device(), &cmdBufAllocateInfo, &copyCmd));
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
		VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));

		// Transition destination image to transfer destination layout
		vks::tools::insertImageMemoryBarrier(
				copyCmd,
				dstImage,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

		// colorAttachment.image is already in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, and does not need to be transitioned

		VkImageCopy imageCopyRegion{};
		imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.srcSubresource.layerCount = 1;
		imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopyRegion.dstSubresource.layerCount = 1;
		imageCopyRegion.extent.width = fsize.width;
		imageCopyRegion.extent.height = fsize.height;
		imageCopyRegion.extent.depth = 1;

		vkCmdCopyImage(
				copyCmd,
				colorAttachment.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageCopyRegion);

		// Transition destination image to general layout, which is the required layout for mapping the image memory later on
		vks::tools::insertImageMemoryBarrier(
				copyCmd,
				dstImage,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });

		VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

		device.submit_work( copyCmd );

		// Get layout of the image (including row pitch)
		VkImageSubresource subResource{};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		VkSubresourceLayout subResourceLayout;

		vkGetImageSubresourceLayout(device.device(), dstImage, &subResource, &subResourceLayout);

		// Map image memory so we can start copying from it
		vkMapMemory(device.device(), dstImageMemory, 0, VK_WHOLE_SIZE, 0, (void**)&imagedata);
		imagedata += subResourceLayout.offset;

		/*
		   Save host visible framebuffer image to disk (ppm format)
		   */


		// If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
		// Check if source is BGR and needs swizzle
		std::vector<VkFormat> formatsBGR = { VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SNORM };
		const bool colorSwizzle = (std::find(formatsBGR.begin(), formatsBGR.end(), VK_FORMAT_R8G8B8A8_UNORM) != formatsBGR.end());

		cv::Mat overlay_internal( cv::Size(fsize.width,fsize.height), CV_8UC4, reinterpret_cast<void*>( imagedata ) );

		cv::Mat overlay;
		overlay_internal.copyTo(overlay);
			
		// Clean up resources
		vkUnmapMemory(device.device(), dstImageMemory);
		vkFreeMemory(device.device(), dstImageMemory, nullptr);
		vkDestroyImage(device.device(), dstImage, nullptr);

		vkQueueWaitIdle(device.get_queue());

		return overlay;
	}

}
