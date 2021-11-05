
#include <AGE/Renderer/Vulkan/VulkanPipelineSystem.h>
#include <AGE/Renderer/Vulkan/VulkanSystem.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>

namespace age::vk
{

constexpr const char k_tag[] = "VulkanPipelineSystem";

PipelineSystem PipelineSystem::s_inst = PipelineSystem();

PipelineHandle PipelineSystem::createPipeline(const Pipeline::CreateInfo &info)
{
	// TODO: Be more clear about what Pipeline are we making as soon as it gets a name
	age_log(k_tag, "Creating Vulkan Pipeline.");

	VkDevice device = VulkanSystem::s_inst.device();

	Pipeline pipeline = {};
	pipeline.createInfo = info;

	// Shader Stages
	StackArray<VkPipelineShaderStageCreateInfo, static_cast<u32>(e_ShaderStage::Count)> shaderStages;
	for (ShaderHandle shaderHandle : info.shaders)
	{
		const Shader &shader = vk::ShaderSystem::s_inst.get(shaderHandle);
		shaderStages.add(createShaderStage(shader));
	}

	{	// createPipelineLayout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// TODO: Properly create the layout

		AGE_VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline.layout));
	}

	{	// createPipeline
		// TODO: Set this up to retrieve an arbitrary mesh instead of hardcoded vertices.
		VkPipelineVertexInputStateCreateInfo vertexInput = {};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = 0;
		vertexInput.pVertexBindingDescriptions = nullptr;
		vertexInput.vertexAttributeDescriptionCount = 0;
		vertexInput.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		const VkExtent2D &swapChainExtent = VulkanSystem::s_inst.swapchainData().extent;
		VkViewport &viewport = pipeline.viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<f32>(swapChainExtent.width);
		viewport.height = static_cast<f32>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		pipeline.scissor.offset = {0, 0};
		pipeline.scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &pipeline.viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &pipeline.scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// TODO: Enable multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// TODO: Handle color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<u32>(shaderStages.count());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInput;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipeline.layout;
		pipelineInfo.renderPass = VulkanSystem::s_inst.renderPass();
		pipelineInfo.subpass = 0;
		
		AGE_VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline));
	}
	
	PipelineHandle handle;
	_pipelinesMap.add(handle, pipeline);

	return handle;
}

void PipelineSystem::cleanup()
{
	age_log(k_tag, "Cleaning up pipelines.");

	for (auto &pipelineNode : _pipelinesMap.asRange())
	{
		if (!pipelineNode.isValid())
			continue;

		const Pipeline &pipeline = pipelineNode.value;
		VkDevice device = VulkanSystem::s_inst.device();

		vkDestroyPipeline(device, pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
	}

	_pipelinesMap.clear();
}

VkPipelineShaderStageCreateInfo PipelineSystem::createShaderStage(const Shader &shader)
{
	VkPipelineShaderStageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.module = shader.module;
	info.stage = shader.stage;
	info.pName = "main";

	return info;
}

}