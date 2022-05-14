#pragma once

#include <vector>
#include "VulkanTools.h"
#include "base_tools.hpp"
#include "v_device.hpp"

namespace vengine
{
	class v_mesh
	{
		private:
			v_device& device;

			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;
	
			//std::vector<VkShaderModule> shaderModules;
			bool data_loadad = false;
			VkBuffer vertexBuffer, indexBuffer;
			VkDeviceMemory vertexMemory, indexMemory;

		public:
			v_mesh( v_device& d, bool normalize );
			~v_mesh();

			void from_obj_file( std::string );
			void from_obj_memory( std::vector<char> &obj_data, std::vector<char> &mtl_data );

			void prepare_vertices_and_indices();

			v_mesh( const v_mesh& ) = delete;
            void operator = ( const v_mesh& ) = delete;
            v_mesh( v_mesh&& ) = delete;
            v_mesh operator = ( v_mesh&& ) = delete;

			VkBuffer& get_vertex_buffer() { return vertexBuffer; };
			VkBuffer& get_index_buffer() { return indexBuffer; };
			
			void bind( VkCommandBuffer& );
			uint32_t count();
	};
};
