#pragma once

#include <Core/DArray.hpp>
#include <Core/Handle.hpp>
#include <Core/Math/Vec3.hpp>
#include <Core/StackArray.hpp>

#include <AGE/Renderer/Vulkan/VulkanMemory.h>

struct GLFWwindow;

namespace age::vk
{

DECLARE_HANDLE(ShaderHandle);
DECLARE_HANDLE(PipelineHandle);
DECLARE_HANDLE(DrawCommandHandle);
DECLARE_HANDLE(MeshHandle);

enum class e_ShaderStage : u8
{
	Vertex,
	Fragment,

	Count
};



struct Vertex
{
	math::vec3 pos;
};



using ShaderArray = StackArray<ShaderHandle, static_cast<u32>(e_ShaderStage::Count)>;

struct PipelineCreateInfo
{
	ShaderArray shaders;
};



void init(GLFWwindow *window);
void cleanup();
void recreateRenderEnvironment();
void draw(const DrawCommandHandle &commandHandle);
void waitForFramesToFinish();

ShaderHandle createShader(e_ShaderStage shaderStage, const char *path);
PipelineHandle createPipeline(const PipelineCreateInfo &info);

DrawCommandHandle createDrawCommand(const PipelineHandle &pipelineHandle, const MeshHandle &meshHandle);
void cleanupDrawCommand(DrawCommandHandle &commandHandle);

MeshHandle createMesh(const DArray<Vertex> &vertices);
void cleanupMesh(MeshHandle &meshHandle);

} // namespace age::vk