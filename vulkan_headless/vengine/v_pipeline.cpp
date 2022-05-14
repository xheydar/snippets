#include "v_pipeline.hpp"

namespace vengine
{	
	VkShaderModule v_pipeline::load_shader( std::vector<char>& shaderCode )
	{
		VkShaderModule shaderModule;
		VkShaderModuleCreateInfo moduleCreateInfo{};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.codeSize = shaderCode.size();
		moduleCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());

		VK_CHECK_RESULT(vkCreateShaderModule(device.device(), &moduleCreateInfo, NULL, &shaderModule));

		return shaderModule;
	}

	void v_pipeline::create_graphics_pipeline( std::vector<char> &shaderCodeVert,
											   std::vector<char> &shaderCodeFrag )
	{	
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(device.device(), &pipelineCacheCreateInfo, nullptr, &pipelineCache));

		// Create pipeline
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

		VkPipelineRasterizationStateCreateInfo rasterizationState =
			vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

		VkPipelineColorBlendAttachmentState blendAttachmentState =
			vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

		VkPipelineColorBlendStateCreateInfo colorBlendState =
			vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);

		VkPipelineViewportStateCreateInfo viewportState =
			vks::initializers::pipelineViewportStateCreateInfo(1, 1);

		VkPipelineMultisampleStateCreateInfo multisampleState =
			vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState =
			vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

		
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].pName = "main";
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].pName = "main";

		shaderStages[0].module = load_shader( shaderCodeVert );
		shaderStages[1].module = load_shader( shaderCodeFrag );
		shaderModules = { shaderStages[0].module, shaderStages[1].module };

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vks::initializers::pipelineCreateInfo(pipelineLayout, renderPass);

		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();

		// Vertex bindings an attributes
		// Binding description
		//
		auto vertex_binding_description = Vertex::get_binding_description();

		// Attribute descriptions
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = Vertex::get_attribute_descriptions(); 

		//auto bindingDescription = Vertex::get_binding_description();
		//auto vertexInputAttributes = Vertex::get_attribute_description();

		VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertex_binding_description;
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

		pipelineCreateInfo.pVertexInputState = &vertexInputState;

		// TODO: There is no command line arguments parsing (nor Android settings) for this
		// example, so we have no way of picking between GLSL or HLSL shaders.
		// Hard-code to glsl for now.
		//const std::string shadersPath = getAssetPath() + "shaders/glsl/renderheadless/";

		
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.device(), pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
	}

	v_pipeline::v_pipeline( v_device& d, std::unique_ptr<v_engine> &e, frame_size s,
							std::vector<char> &shaderCodeVert, 
							std::vector<char> &shaderCodeFrag ):device{d}, engine{e}, fsize{s}
	{
		colorFormat = engine -> get_colorFormat();
		depthFormat = engine -> get_depthFormat();
		framebuffer = engine -> get_framebuffer();
		colorAttachment = engine -> get_colorAttachment();
		depthAttachment = engine -> get_depthAttachment();
		renderPass = engine -> get_rende_pass();
		descriptorSetLayout = engine -> get_descriptSetLayout();
		pipelineLayout = engine -> get_pipelineLayout();

		//shader_vert_path = cfg["vert"];
		//shader_frag_path = cfg["frag"];
		
		create_graphics_pipeline( shaderCodeVert, shaderCodeFrag );
	}

	v_pipeline::~v_pipeline()
	{
		for (auto shadermodule : shaderModules)
		{
			vkDestroyShaderModule(device.device(), shadermodule, nullptr);
		}
		vkDestroyPipeline(device.device(), pipeline, nullptr);
		vkDestroyPipelineCache(device.device(), pipelineCache, nullptr);
	}

	void v_pipeline::bind( VkCommandBuffer &commandBuffer )
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
}
