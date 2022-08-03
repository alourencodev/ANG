#pragma once

#include <Core/SArray.hpp>

#include <AGE/Renderer/Vulkan/VkContext.h>
#include <AGE/Renderer/Vulkan/VkFrameSync.h>
#include <AGE/Renderer/Vulkan/VkPipeline.h>
#include <AGE/Renderer/Vulkan/VkRenderTarget.h>
#include <AGE/Renderer/Vulkan/VkShader.h>
#include <AGE/Renderer/Vulkan/VkSwapchain.h>

#include <Core/DArray.hpp>
#include <Core/SArray.hpp>
#include <Core/Math/Vec3.hpp>


struct GLFWwindow;

namespace age
{

class Renderer
{
	static const u8 k_maxFramesInFlight = 3;

public:
	static Renderer s_inst;

	void init(GLFWwindow *window);
	void draw();
	void cleanup();

	void setClearColot(math::color3 clearColor) { _clearValues[static_cast<u32>(EClearValueType::Color)].color = {{clearColor.r, clearColor.g, clearColor.b, 1.0f}}; }

	void onWindowResize(int width, int height);

private:
	enum class EClearValueType : u8
	{
		Color,
		Depth, 

		Count
	};

	void submit(VkCommandBuffer &commandBuffer);
	void present(u32 imageIndex);
	void reacreateSwapchain();

	vk::Context _context;
	vk::Swapchain _swapchain;
	vk::RenderTarget _renderTarget;
	SArray<vk::FrameSync, k_maxFramesInFlight> _frameSyncArray;
	u8 _currentFrame = 0;
	math::sizei _windowSize;

	// Resources
	DArray<vk::Shader> _shaderArray;
	DArray<vk::Pipeline> _pipelineArray;
	SArray<VkCommandBuffer, k_maxFramesInFlight> _commandBuffers;
	SArray<VkClearValue, static_cast<size_t>(EClearValueType::Count)> _clearValues;
};

}
