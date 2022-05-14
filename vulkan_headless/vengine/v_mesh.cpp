#include "v_mesh.hpp"

namespace vengine
{
	void v_mesh::prepare_vertices_and_indices()
	{
		const VkDeviceSize vertexBufferSize = vertices.size() * sizeof(Vertex);
		const VkDeviceSize indexBufferSize = indices.size() * sizeof(uint32_t);

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		auto commandPool = device.get_command_pool();

		// Command buffer for copy commands (reused)
		VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(commandPool, 
																					VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		VkCommandBuffer copyCmd;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(device.device(), &cmdBufAllocateInfo, &copyCmd));
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		// Copy input data to VRAM using a staging buffer
		// Vertices
		device.createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&stagingBuffer,
				&stagingMemory,
				vertexBufferSize,
				vertices.data());

		device.createBuffer(
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&vertexBuffer,
				&vertexMemory,
				vertexBufferSize);

		VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
		VkBufferCopy copyRegion = {};
		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(copyCmd, stagingBuffer, vertexBuffer, 1, &copyRegion);
		VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

		device.submit_work( copyCmd );

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingMemory, nullptr);

		// Indices
		device.createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&stagingBuffer,
				&stagingMemory,
				indexBufferSize,
				indices.data());

		device.createBuffer(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				&indexBuffer,
				&indexMemory,
				indexBufferSize);

		VK_CHECK_RESULT(vkBeginCommandBuffer(copyCmd, &cmdBufInfo));
		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(copyCmd, stagingBuffer, indexBuffer, 1, &copyRegion);
		VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

		device.submit_work( copyCmd );

		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingMemory, nullptr);

		data_loadad = true;
	}

	v_mesh::v_mesh( v_device &d,
					bool normalize
					) : device{d}
	{
	}

	void v_mesh::from_obj_file( std::string path )
	{
		auto [ vs, is ] = tools::load_obj_file( path );
		vertices = vs;
		indices = is;	
	}

	void v_mesh::from_obj_memory( std::vector<char>& obj_data, std::vector<char>& mtl_data )
	{
		std::string obj_str( obj_data.begin(), obj_data.end() );
		std::string mtl_str( mtl_data.begin(), mtl_data.end() );

		auto [ vs, is ] = tools::load_obj_memory( obj_str, mtl_str );
		vertices = vs;
		indices = is;
	}

	v_mesh::~v_mesh()
	{
		if( data_loadad )
		{
			vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
			vkFreeMemory(device.device(), vertexMemory, nullptr);
			vkDestroyBuffer(device.device(), indexBuffer, nullptr);
			vkFreeMemory(device.device(), indexMemory, nullptr);
		}
	}

	void v_mesh::bind( VkCommandBuffer& commandBuffer )
	{
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	uint32_t v_mesh::count()
	{
		return static_cast<uint32_t>(indices.size());
	}
}
