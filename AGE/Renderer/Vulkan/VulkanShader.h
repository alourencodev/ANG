#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>
#include <Core/Handle.hpp>
#include <Core/SArray.hpp>


namespace age::vk
{

DECLARE_HANDLE(ShaderHandle);


struct Context;

enum class EShaderStage : u8
{
	Vertex,
	Fragment,

	Count
};

// Also use count as invalid
constexpr EShaderStage k_invalidShaderStage = EShaderStage::Count;

using ShaderHandleArray = SArray<ShaderHandle, static_cast<u8>(EShaderStage::Count)>;



struct Shader
{
	VkAllocationCallbacks *allocator = nullptr;
	VkShaderModule module;
	VkShaderStageFlagBits stage;
};


struct ShaderCreateInfo
{
	VkAllocationCallbacks *allocator = nullptr;
	const DArray<byte> *source = nullptr;
	EShaderStage stage = k_invalidShaderStage;
};


Shader createShader(const Context &context, const ShaderCreateInfo &info);
void destroyShader(const Context &context, Shader &shader);

}	// namespace age::vk
