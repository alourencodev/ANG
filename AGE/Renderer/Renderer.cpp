#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/Vulkan.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>
#include <AGE/Vendor/GLFW.hpp>

namespace age
{

const DArray<vk::Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}},
    {{0.5f, 0.5f, 1.0f}},
    {{-0.5f, 0.5f, 1.0f}}
};



const DArray<u32> indices = {
	0, 1, 2, 2 ,3 ,0
};



Renderer Renderer::s_inst = Renderer();

constexpr char k_tag[] = "Renderer";

vk::ShaderHandle vertexShader;
vk::ShaderHandle fragmentShader;

vk::DrawCommandHandle testDrawCommand;
vk::MeshHandle testMeshBuffer;

void Renderer::init(GLFWwindow *window)
{
	{	// Create Vulkan Context
		vk::ContextCreateInfo createInfo = {};
		createInfo.appName = "ANG";
		createInfo.window = window;

		_context = vk::createContext(createInfo);
	}


	{	// Create Vulkan Swpchain
		i32 width, height;
		glfwGetFramebufferSize(window, &width, &height);
		_windowSize = {static_cast<u32>(width), static_cast<u32>(height)};

		vk::SwapchainCreateInfo createInfo = {};
		createInfo.framebufferSize = _windowSize;

		_swapchain = vk::createSwapchain(_context, createInfo);
	}

	_renderTarget = vk::createRenderTarget(_context, _swapchain, nullptr);

	for (vk::FrameSync &frameSync : _frameSyncArray)
		frameSync = vk::createFrameSync(_context, nullptr);

	//vertexShader = vk::createShader(vk::e_ShaderStage::Vertex, "Shaders/dummy.vert.spv");
	//fragmentShader = vk::createShader(vk::e_ShaderStage::Fragment, "Shaders/dummy.frag.spv");

	//vk::PipelineCreateInfo info = {};
	//info.shaders.add(vertexShader);
	//info.shaders.add(fragmentShader);
	//vk::MeshPipelineHandle pipelineHandle = vk::createMeshPipeline(info);

	//testMeshBuffer = vk::createMesh(vertices, indices);
	//testDrawCommand = vk::createDrawCommand(pipelineHandle, testMeshBuffer);
}

void Renderer::draw()
{
	age_assertFatal(_currentFrame < k_maxFramesInFlight, "Current frame out of bounds.");

	// Make sure this frame is done
	vk::FrameSync &frameSync = _frameSyncArray[_currentFrame];
	vkWaitForFences(_context.device, 1, &frameSync.inFlightFence, VK_TRUE, UINT64_MAX);

	u32 imageIndex;
	{	// Acquire Next Swapchain Image
		VkResult result = vkAcquireNextImageKHR(_context.device, _swapchain.swapchain, UINT64_MAX, frameSync.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			reacreateSwapchain();			
		else
			AGE_VK_CHECK(result);
	}


	{	// Sycnc Image
		VkFence &imageInFlightFence = _swapchain.imageInFlightFences[imageIndex];
		if (imageInFlightFence != VK_NULL_HANDLE)
			vkWaitForFences(_context.device, 1u, &imageInFlightFence, VK_TRUE, UINT64_MAX);

		imageInFlightFence = frameSync.inFlightFence;
	}

	// TODO: Create Command Buffer
	// TODO: Submit

	present(imageIndex);

	_currentFrame = (_currentFrame + 1) % k_maxFramesInFlight;
}



void Renderer::submit(VkCommandBuffer &commandBuffer)
{
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	const VkQueue graphicsQueue = _context.queues[static_cast<i32>(vk::EQueueFamily::Graphics)];
	const vk::FrameSync &frameSync = _frameSyncArray[_currentFrame];

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &frameSync.imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &frameSync.renderFinishedSemaphore;

	vkResetFences(_context.device, 1, &frameSync.inFlightFence);
	AGE_VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, frameSync.inFlightFence));
}



void Renderer::present(u32 imageIndex)
{
	const VkQueue presentQueue = _context.queues[static_cast<i32>(vk::EQueueFamily::Presentation)];
	const vk::FrameSync &frameSync = _frameSyncArray[_currentFrame];

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &frameSync.renderFinishedSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain.swapchain;
	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
		reacreateSwapchain();
	else
		AGE_VK_CHECK(result);

	vkQueueWaitIdle(presentQueue);
}



void Renderer::cleanup()
{
	for (vk::FrameSync &frameSync : _frameSyncArray)
		vk::destroyFrameSync(_context, frameSync);

	vk::destroyRenderTarget(_context, _renderTarget);
	vk::destroySwapchain(_context, _swapchain);
	vk::destroyContext(_context);
}



void Renderer::onWindowResize(GLFWwindow */*window*/, int width, int height)
{
	age_log(k_tag, "Window resized to %d x %d.", width, height);

	_windowSize = {static_cast<u32>(width), static_cast<u32>(height)};
	reacreateSwapchain();
}



void Renderer::reacreateSwapchain()
{
	vkDeviceWaitIdle(_context.device);

	VkAllocationCallbacks *swapchainAllocator = _swapchain.allocator;
	VkAllocationCallbacks *renderTargetAllocator = _renderTarget.allocator;

	vk::destroyRenderTarget(_context, _renderTarget);
	vk::destroySwapchain(_context, _swapchain);

	vk::SwapchainCreateInfo swapchainInfo = {};
	swapchainInfo.allocator = swapchainAllocator;
	swapchainInfo.framebufferSize = _windowSize;
	_swapchain = vk::createSwapchain(_context, swapchainInfo);

	_renderTarget = vk::createRenderTarget(_context, _swapchain, renderTargetAllocator);
}


}