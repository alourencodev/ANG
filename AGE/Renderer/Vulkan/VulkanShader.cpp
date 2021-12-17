
#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanShader.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>


namespace age::vk
{

Shader createShader(const Context &context, const ShaderCreateInfo &info)
{
	age_assertFatal(info.source != nullptr, "Trying to create shader with no source.");
	age_assertFatal(info.stage != k_invalidShaderStage, "Trying to create shader with invalid stage");

	Shader shader = {};
	shader.allocator = info.allocator;

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.codeSize = static_cast<u32>(info.source->count());
	createInfo.pCode = reinterpret_cast<const u32 *>(info.source->data());

	AGE_VK_CHECK(vkCreateShaderModule(context.device, &createInfo, shader.allocator, &shader.module));

	switch (info.stage)
	{
		case EShaderStage::Vertex: {
			shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		}
		case EShaderStage::Fragment: {
			shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		}
		default: {
			age_assertFatal(false, "Trying to create a shader with an unhandled shader stage");
		}
	}

	return shader;
}



void destroyShader(const Context &context, Shader &shader)
{
	vkDestroyShaderModule(context.device, shader.module, shader.allocator);
}

}	// namespace age::vk