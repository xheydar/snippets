#pragma once

#include <iostream>
#include <string>
#include <chrono>


#include "v_device.hpp"
#include "v_pipeline.hpp"
#include "base_tools.hpp"
#include "v_mesh.hpp"
#include "v_texture.hpp"

namespace vengine
{
	class v_object
	{
		private:
			std::unique_ptr<v_mesh>& mesh;	
			int pool_idx_;
			std::string pipeline_name_;

			bool always_alive = false;
			int seconds_alive = 10;
			std::string pretty_name = "";
			int token_value = 100;

			std::chrono::time_point<std::chrono::system_clock> birth_timestamp;
		public:
			v_object( std::unique_ptr<v_mesh>& m,
							  int pool_idx,
							  std::string );
			~v_object();

			// Vulkan modules
			void bind( VkCommandBuffer& );
			uint32_t count();
			std::string pipeline_name() { return pipeline_name_; };
			int& pool_idx() { return pool_idx_; };

			// Functions to make objects visible
			void set_visibility( bool aa, int sa, std::string pn, int tv );
			bool is_alive();			
			void revive();
			std::string name() { return pretty_name; };
			int tokens() { return token_value; };
	};
}
