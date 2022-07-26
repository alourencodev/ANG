
# ShaderCompiler CMakeFile

cmake_minimum_required(VERSION 3.22)

project(ShaderCompiler)
message(" === Project: ${PROJECT_NAME}")

# Import shaderc
add_library(shaderc STATIC IMPORTED)
set_target_properties(shaderc PROPERTIES
	IMPORTED_LOCATION "C:/VulkanSDK/Lib/shaderc_combined.lib"
	IMPORTED_LOCATION_DEBUG "C:/VulkanSDK/Lib/shaderc_combinedd.lib"
)

set(SHADERC_INCLUDE_DIR "C:/VulkanSDK/Include")

file(GLOB_RECURSE SC_SOURCE "*.h" "*.hpp" "*.cpp")

add_executable(ShaderCompiler ${SC_SOURCE})

set_target_properties(ShaderCompiler PROPERTIES OUTPUT_NAME_DEBUG "${PROJECT_NAME}_d")

target_include_directories(ShaderCompiler PRIVATE ${CMAKE_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/Vendor" "${SHADERC_INCLUDE_DIR}")
target_link_libraries(ShaderCompiler PRIVATE Core shaderc)
target_compile_options(ShaderCompiler PRIVATE ${AGE_COMPILE_OPTIONS})
target_compile_definitions(ShaderCompiler PRIVATE ${AGE_BUILD_CONFIG})

