
#include <AGE/Renderer/Vulkan/VulkanMesh.h>

#include <Core/Log/Log.h>


namespace age::vk
{

Mesh createMesh(const Context &context, const DArray<Vertex> &vertexArray, const DArray<u32> &indexArray, VkAllocationCallbacks *allocator /*= nullptr*/)
{
	age_assertFatal(!vertexArray.isEmpty(), "Unable to create a mesh with no vertices.");
	age_assertFatal(!indexArray.isEmpty(), "Unable to create a mesh with no indices.");

	VkMemoryPropertyFlags commonFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	Mesh mesh;
	mesh.indexCount = static_cast<u32>(indexArray.count());

	// Vertex Buffer
	const size_t vertexBufferSize = sizeof(Vertex) * vertexArray.count();
	mesh.vertices = allocBuffer(context, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | commonFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator);
	stageBuffer(context, mesh.vertices, vertexArray.data(), vertexBufferSize);

	// Index Buffer
	const size_t indexBufferSize = sizeof(u32) * indexArray.count();
	mesh.indices = allocBuffer(context, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | commonFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocator);
	stageBuffer(context, mesh.indices, indexArray.data(), indexBufferSize);

	return mesh;
}



void destroyMesh(const Context &context, Mesh &mesh)
{
	freeBuffer(context, mesh.indices);
	freeBuffer(context, mesh.vertices);

	mesh.indexCount = 0;
}

}	// nmespace age::vk