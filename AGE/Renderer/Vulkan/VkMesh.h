#pragma once

#include <AGE/Renderer/Vulkan/VkMemory.h>
#include <AGE/Renderer/Vertex.hpp>

#include <Core/DArray.hpp>


namespace age::vk
{

struct Context;



struct Mesh
{
	Buffer vertices;
	Buffer indices;
	u32 indexCount = 0;
};



Mesh createMesh(const Context &context, const DArray<Vertex> &vertexArray, const DArray<u32> &indexArray, VkAllocationCallbacks *allocator = nullptr);
void destroyMesh(const Context &context, Mesh &mesh);

}	// namespace age::vk
