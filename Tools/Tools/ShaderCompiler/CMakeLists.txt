
# ShaderCompiler CMakeFile

cmake_minimum_required(VERSION 3.22)

project(ShaderCompiler)
message(" === Project: ${PROJECT_NAME}")

# Import shaderc
add_library(shaderc SHARED IMPORTED)
set_target_properties(shaderc PROPERTIES
	IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/Lib/shaderc/lib/shaderc.lib"
)

file(GLOB_RECURSE SC_SOURCE "*.h" "*.hpp" "*.cpp")

add_executable(ShaderCompiler ${SC_SOURCE})

target_include_directories(ShaderCompiler PRIVATE ${CMAKE_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/Vendor")
target_link_libraries(ShaderCompiler PRIVATE Core shaderc)
target_compile_options(ShaderCompiler PRIVATE ${AGE_COMPILE_OPTIONS})
target_compile_definitions(ShaderCompiler PRIVATE ${AGE_BUILD_CONFIG})

