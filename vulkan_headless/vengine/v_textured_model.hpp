#pragma once

#include <iostream>
#include <string>

#include "v_device.hpp"
#include "v_pipeline.hpp"
#include "base_tools.hpp"
#include "v_mesh.hpp"
#include "v_texture.hpp"

namespace vengine
{
	class v_textured_model
	{
		private:
			std::unique_ptr<v_mesh>& mesh;	
			int pool_idx_;
			std::string pipeline_name_;
		public:
			v_textured_model( std::unique_ptr<v_mesh>& m,
							  int pool_idx,
							  std::string );
			~v_textured_model();

			void bind( VkCommandBuffer& );
			uint32_t count();
			std::string pipeline_name() { return pipeline_name_; };
			int& pool_idx() { return pool_idx_; };
	};
}
