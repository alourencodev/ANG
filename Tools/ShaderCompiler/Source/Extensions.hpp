#pragma once

#include <shaderc/shaderc.h>

#include <Core/String.hpp>
#include <Core/HashMap.hpp>



namespace age
{

// Input Extensions
//
// They are strings an not char[] for convenient comparison inside container
static const String k_vertexExtension = ".vert";
static const String k_fragmentExtension = ".frag";
static const String k_glslExtension = ".glsl";		// Non stage specific headers might use this extension



static const age::HashMap<String, shaderc_shader_kind> k_extensionMap = {
	{k_vertexExtension, shaderc_vertex_shader},
	{k_fragmentExtension, shaderc_fragment_shader}
};



static const age::SArray<String, 3> k_validExtensions = {
	k_vertexExtension,
	k_fragmentExtension,
	k_glslExtension
};

}	// namespace age
