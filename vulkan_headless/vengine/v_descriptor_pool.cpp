#include "v_descriptor_pool.hpp"

namespace vengine
{
	v_descriptor_pool::v_descriptor_pool( v_device &d, std::unique_ptr<v_engine>& e,
										  std::map<std::string,std::unique_ptr<v_texture>> &textures ) : device{d}, engine{e}
	{
		auto descriptorSetLayout = engine -> get_descriptSetLayout();

		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		//poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//poolSizes[0].descriptorCount = 1;
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(textures.size());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(textures.size());

		VK_CHECK_RESULT( vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool) );

		std::vector<VkDescriptorSetLayout> layouts(textures.size(), descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(textures.size()); 
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize( textures.size() );

		VK_CHECK_RESULT( vkAllocateDescriptorSets(device.device(), &allocInfo, descriptorSets.data()) );

		/*
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkWriteDescriptorSet uniformBufferDescriptorWrite{};
		uniformBufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformBufferDescriptorWrite.dstSet = descriptorSet;
		uniformBufferDescriptorWrite.dstBinding = 0;
		uniformBufferDescriptorWrite.dstArrayElement = 0;
		uniformBufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferDescriptorWrite.descriptorCount = 1;
		uniformBufferDescriptorWrite.pBufferInfo = &bufferInfo;
		uniformBufferDescriptorWrite.pImageInfo = nullptr; // Optional
		uniformBufferDescriptorWrite.pTexelBufferView = nullptr; // Optional
		*/

		int idx = 0;
		for( const auto &texture : textures )
		{
			// Setup a descriptor image info for the current texture to be used as a combined image sampler

			VkDescriptorImageInfo textureDescriptor;
			textureDescriptor.imageView = texture.second -> get_image_view();
			textureDescriptor.sampler = texture.second -> get_texture_sampler();
			textureDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet samplerBufferDescriptorWrite{};
			samplerBufferDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			samplerBufferDescriptorWrite.dstSet = descriptorSets[idx];
			samplerBufferDescriptorWrite.dstBinding = 0;
			samplerBufferDescriptorWrite.dstArrayElement = 0;
			samplerBufferDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerBufferDescriptorWrite.descriptorCount = 1;
			samplerBufferDescriptorWrite.pImageInfo = &textureDescriptor; // Optional

			std::vector<VkWriteDescriptorSet> writeDescriptorSets = 
			{
				//uniformBufferDescriptorWrite,
				samplerBufferDescriptorWrite
			};

			vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(writeDescriptorSets.size()), 
					writeDescriptorSets.data(), 0, nullptr);

			texture.second -> pool_idx() = idx;

			idx ++;
		}
	}

	v_descriptor_pool::~v_descriptor_pool()
	{
		vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
	}

	void v_descriptor_pool::bind( VkCommandBuffer& commandBuffer, int setIdx )
	{	
		if( setIdx >= 0 )
		{
			auto pipelineLayout = engine -> get_pipelineLayout();
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
									pipelineLayout, 0, 1, &descriptorSets[setIdx], 0, 0);
		}
	}
}
