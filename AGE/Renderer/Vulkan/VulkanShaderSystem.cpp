
#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>
#include <AGE/Renderer/Vulkan/VulkanSystem.h>

#include <Core/File.h>


namespace age::vk
{

constexpr const char k_tag[] = "VulkanShaderSystem";

ShaderSystem ShaderSystem::s_inst = ShaderSystem();

ShaderHandle ShaderSystem::createShader(e_ShaderStage shaderStage, const char *path)
{
	age_log(k_tag, "Creating shader from %s", path);

	// TODO: Check if we can discard the source after the module gets created.
	//		 Otherwise we cant's clean 'source' at the end of the scope.

	DArray<char> source = file::readBinary(path);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.codeSize = static_cast<u32>(source.count());
	createInfo.pCode = reinterpret_cast<const u32 *>(source.data());

	Shader shader{};
	AGE_VK_CHECK(vkCreateShaderModule(VulkanSystem::s_inst.device(), &createInfo, nullptr, &shader.module));

	switch (shaderStage)
	{
		case e_ShaderStage::Vertex: {
			shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		}
		case e_ShaderStage::Fragment: {
			shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		}
		default: {
			age_assertFatal(false, "Trying to create a shader with an unhandled shader stage");
		}
	}

	ShaderHandle handle(static_cast<u32>(_shaders.count()));
	_shaders.add(shader);

	return handle;
}

void ShaderSystem::cleanup()
{
	age_log(k_tag, "Cleaning up shaders");

	for (Shader &shader : _shaders) {
		vkDestroyShaderModule(VulkanSystem::s_inst.device(), shader.module, nullptr);
	}

	_shaders.clear();
}

}