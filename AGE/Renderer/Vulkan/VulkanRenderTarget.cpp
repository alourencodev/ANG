#include <AGE/Renderer/Vulkan/VulkanRenderTarget.h>

#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanSwapchain.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>



namespace age::vk
{

constexpr const char k_tag[] = "VulkanBootstrap";



void createRenderPass(const Context &context, const Swapchain &swapchain, RenderTarget &renderTarget)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapchain.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;	// Multisample is disabled
	// TODO: Change this to LOAD_OP_DONT_CARE when we have a whole scene being drawn
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// We want the render pass to start, even if there is no image available.
	// Therefore, we need to set up a subpass dependency in order to go further
	// until the color attachment output stage
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	AGE_VK_CHECK(vkCreateRenderPass(context.device, &renderPassInfo, renderTarget.allocator, &renderTarget.renderPass));
}



void createFramebuffers(const Context &context, const Swapchain &swapchain, RenderTarget &renderTarget)
{
	const auto &imageViews = swapchain.imageViews;
	renderTarget.framebuffers.reserveWithEmpty(imageViews.count());

	for (int i = 0; i < imageViews.count(); i++) {
		VkImageView attachments[] = {imageViews[i]};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderTarget.renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchain.extent.width;
		framebufferInfo.height = swapchain.extent.height;
		framebufferInfo.layers = 1;
		
		AGE_VK_CHECK(vkCreateFramebuffer(context.device, &framebufferInfo, renderTarget.allocator, &(renderTarget.framebuffers[i])));
	}

	age_log(k_tag, "Created %d framebuffers", renderTarget.framebuffers.count());
}



RenderTarget createRenderTarget(const Context &context, const Swapchain &swapchain, VkAllocationCallbacks *allocator)
{
	RenderTarget renderTarget = {};
	renderTarget.allocator = allocator;

	createRenderPass(context, swapchain, renderTarget);
	createFramebuffers(context, swapchain, renderTarget);

	return renderTarget;
}



void destroyRenderTarget(const Context &context, RenderTarget &renderTarget)
{
	for (auto framebuffer : renderTarget.framebuffers)
		vkDestroyFramebuffer(context.device,framebuffer, renderTarget.allocator);
	renderTarget.framebuffers.clear();

	vkDestroyRenderPass(context.device, renderTarget.renderPass, renderTarget.allocator);
}

}	// namespace age::vk