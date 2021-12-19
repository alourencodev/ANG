#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/VulkanMesh.h>
#include <AGE/Renderer/Vulkan/VulkanShader.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>
#include <AGE/Renderer/Vertex.hpp>
#include <AGE/Vendor/GLFW.hpp>

#include <Core/File.h>


namespace age
{

const DArray<Vertex> vertices = {
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

static const VkClearValue k_defaultClearColor = {{0.2f, 0.2f, 0.2f, 1.0f}};
constexpr VkClearValue k_defaultClearDepth = {{1.0f, 0}};

vk::ShaderHandle vertexShader;
vk::ShaderHandle fragmentShader;

vk::PipelineHandle testPipeline;

vk::Mesh testMesh;



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


	{	// Allocate Command Buffers
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandPool = _context.graphicsCommandPool;
		info.commandBufferCount = k_maxFramesInFlight;

		AGE_VK_CHECK(vkAllocateCommandBuffers(_context.device, &info, _commandBuffers.data()));
	}


	_renderTarget = vk::createRenderTarget(_context, _swapchain, nullptr);

	_clearValues[static_cast<u32>(EClearValueType::Color)] = k_defaultClearColor;
	_clearValues[static_cast<u32>(EClearValueType::Depth)] = k_defaultClearDepth;

	for (vk::FrameSync &frameSync : _frameSyncArray)
		frameSync = vk::createFrameSync(_context, nullptr);


	{	// Placeholders
		const DArray<byte> vertexSource = file::readBinary("Shaders/dummy.vert.spv");
		const DArray<byte> fragmentSource = file::readBinary("Shaders/dummy.frag.spv");

		vk::ShaderCreateInfo vertInfo = {};
		vertInfo.source = &vertexSource;
		vertInfo.stage = vk::EShaderStage::Vertex;
		_shaderArray.add(vk::createShader(_context, vertInfo));
		vertexShader = vk::ShaderHandle(0);

		vk::ShaderCreateInfo fragInfo = {};
		fragInfo.source = &fragmentSource;
		fragInfo.stage = vk::EShaderStage::Fragment;
		_shaderArray.add(vk::createShader(_context, fragInfo));
		fragmentShader = vk::ShaderHandle(1);

		vk::PipelineCreateInfo pipelineInfo = {};
		pipelineInfo.shaderHandles[static_cast<u8>(vk::EShaderStage::Vertex)] = vertexShader;
		pipelineInfo.shaderHandles[static_cast<u8>(vk::EShaderStage::Fragment)] = fragmentShader;

		vk::Pipeline pipeline = {};
		_pipelineArray.add(vk::createPipeline(_context, _swapchain, _renderTarget, _shaderArray, pipelineInfo));
		testPipeline = vk::PipelineHandle(0);

		testMesh = vk::createMesh(_context, vertices, indices);
	}
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

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	cmdBeginInfo.pInheritanceInfo = nullptr;

	VkCommandBuffer currentCmdBuffer = _commandBuffers[_currentFrame];
	// vkBeginCommandBuffer implicitly resets the command buffer
	AGE_VK_CHECK(vkBeginCommandBuffer(currentCmdBuffer, &cmdBeginInfo));
	{
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = _renderTarget.renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = _swapchain.extent;
		renderPassBeginInfo.framebuffer = _renderTarget.framebuffers[imageIndex];
		renderPassBeginInfo.clearValueCount = static_cast<u32>(_clearValues.size());
		renderPassBeginInfo.pClearValues = _clearValues.data();

		vkCmdBeginRenderPass(currentCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	
		{	// Placeholders
			const VkDeviceSize offsets[] = {0};
			vkCmdBindPipeline(currentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineArray[testPipeline].pipeline);
			vkCmdBindVertexBuffers(currentCmdBuffer, 0, 1, &testMesh.vertices.buffer, offsets);
			vkCmdBindIndexBuffer(currentCmdBuffer, testMesh.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(currentCmdBuffer, testMesh.indexCount, 1 ,0, 0, 0);
		}

		// TODO: Draw Objects

		vkCmdEndRenderPass(currentCmdBuffer);
	}
	AGE_VK_CHECK(vkEndCommandBuffer(currentCmdBuffer));

	submit(currentCmdBuffer);
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
	vkDeviceWaitIdle(_context.device);

	{	// Placeholders
		vk::destroyMesh(_context, testMesh);
	}

	for (vk::Pipeline &pipeline : _pipelineArray)
		vk::destroyPipeline(_context, pipeline);

	for (vk::Shader &shader : _shaderArray)
		vk::destroyShader(_context, shader);

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

	// Recreate Pipelines
	for (vk::Pipeline &pipeline : _pipelineArray)
	{
		vk::PipelineCreateInfo info = {};
		info.allocator = pipeline.allocator;
		info.shaderHandles = pipeline.shaderHandles;

		vk::destroyPipeline(_context, pipeline);	
		pipeline = vk::createPipeline(_context, _swapchain, _renderTarget, _shaderArray, info);
	}
}

}