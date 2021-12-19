#pragma once

#include <Core/DArray.hpp>
#include <Core/Handle.hpp>
#include <Core/Math/Vec3.hpp>
#include <Core/StackArray.hpp>

struct GLFWwindow;

namespace age::vk
{

DECLARE_HANDLE(ShaderHandle_Legacy);
DECLARE_HANDLE(MeshPipelineHandle);
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



using ShaderHandleArray_Legacy = StackArray<ShaderHandle_Legacy, static_cast<u32>(e_ShaderStage::Count)>;

struct PipelineCreateInfo_Legacy
{
	ShaderHandleArray_Legacy shaders;
};



void init(GLFWwindow *window);
void cleanup();
void recreateRenderEnvironment();
void draw(const DrawCommandHandle &commandHandle);
void waitForFramesToFinish();

ShaderHandle_Legacy createShader(e_ShaderStage shaderStage, const char *path);
MeshPipelineHandle createMeshPipeline(const PipelineCreateInfo_Legacy &info);

DrawCommandHandle createDrawCommand(const MeshPipelineHandle &pipelineHandle, const MeshHandle &meshHandle);
void cleanupDrawCommand(DrawCommandHandle &commandHandle);

MeshHandle createMesh(const DArray<Vertex> &vertices, const DArray<u32> &indices);
void cleanupMesh(MeshHandle &meshHandle);

} // namespace age::vk