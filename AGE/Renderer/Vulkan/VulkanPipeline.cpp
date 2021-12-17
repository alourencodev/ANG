
#include <AGE/Renderer/Vulkan/VulkanPipeline.h>

#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanRenderTarget.h>
#include <AGE/Renderer/Vulkan/VulkanSwapchain.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>
#include <AGE/Renderer/Vertex.hpp>

#include <Core/StackArray.hpp>


namespace age::vk
{

constexpr const char k_tag[] = "VulkanBootstrap";



VkPipelineShaderStageCreateInfo createShaderStage(const Shader &shader)
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



Pipeline createPipeline(const Context &context, const Swapchain &swapchain, const RenderTarget &renderTarget, const DArray<Shader> &shaders, const PipelineCreateInfo &info)
{
	age_log(k_tag, "Creating Vulkan Pipeline.");

	Pipeline pipeline = {};
	pipeline.allocator = info.allocator;


	// Create Shader Stages
	pipeline.shaderHandles = info.shaderHandles;
	StackArray<VkPipelineShaderStageCreateInfo, static_cast<u32>(EShaderStage::Count)> shaderStages;
	for (ShaderHandle shaderHandle : pipeline.shaderHandles) {
		if (!shaderHandle.isValid())
			continue;

		age_assertFatal(shaderHandle < shaders.count(), "Trying to create pipeline with inexistent shader.");

		shaderStages.add(createShaderStage(shaders[shaderHandle]));
	}


	{	// Create Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// TODO: Properly create the layout

		AGE_VK_CHECK(vkCreatePipelineLayout(context.device, &pipelineLayoutInfo, nullptr, &pipeline.layout));
	}


	{	// Create Pipeline

		// Set vertex Attributes
		constexpr u8 k_attributesPerVertex = 1;
		SArray<VkVertexInputAttributeDescription, k_attributesPerVertex> attributesDescription;
		{
			// pos
			attributesDescription[0].binding = 0;
			attributesDescription[0].location = 0;
			attributesDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributesDescription[0].offset = offsetof(Vertex, pos);
		}

		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo vertexInput = {};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = 1;
		vertexInput.pVertexBindingDescriptions = &bindingDescription;
		vertexInput.vertexAttributeDescriptionCount = k_attributesPerVertex;
		vertexInput.pVertexAttributeDescriptions = attributesDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		const VkExtent2D &swapChainExtent = swapchain.extent;
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
		pipelineInfo.renderPass = renderTarget.renderPass;
		pipelineInfo.subpass = 0;
		
		AGE_VK_CHECK(vkCreateGraphicsPipelines(context.device, VK_NULL_HANDLE, 1, &pipelineInfo, pipeline.allocator, &pipeline.pipeline));
	}

	return pipeline;
}



void destroyPipeline(const Context &context, Pipeline &pipeline)
{
	vkDestroyPipeline(context.device, pipeline.pipeline, pipeline.allocator);
	vkDestroyPipelineLayout(context.device, pipeline.layout, pipeline.allocator);
}

}