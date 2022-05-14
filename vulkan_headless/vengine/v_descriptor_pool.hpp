#pragma once

#include <iostream>
#include "VulkanTools.h"
#include "base_tools.hpp"
#include "v_device.hpp"
#include "v_engine.hpp"
#include "v_pipeline.hpp"
#include "v_texture.hpp"
#include "v_textured_model.hpp"

namespace vengine
{
	class v_descriptor_pool
	{
		private:
			v_device &device;
			std::unique_ptr<v_engine>& engine;

			VkDescriptorPool descriptorPool;
			std::vector<VkDescriptorSet> descriptorSets;
			//VkDescriptorSet descriptorSet;

		public:
			v_descriptor_pool( v_device &d, 
							   std::unique_ptr<v_engine>& e,
							   std::map<std::string,std::unique_ptr<v_texture>> &t );
			~v_descriptor_pool();

			void bind( VkCommandBuffer&, int setIdx );
	};
}
