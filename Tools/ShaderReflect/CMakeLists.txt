
# ShaderReflect CMakeLists.txt

cmake_minimum_required(VERSION 3.22)

project(ShaderReflect)
message(" === Project: ${PROJECT_NAME}")

file(GLOB_RECURSE SHADER_REFLECT_SOURCE "*.h" "*.hpp" "*.cpp")

add_executable(ShaderReflect ${SHADER_REFLECT_SOURCE})

target_include_directories(ShaderReflect PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(ShaderReflect PRIVATE Core yaml-cpp)
target_compile_options(ShaderReflect PRIVATE ${AGE_COMPILE_OPTIONS})
target_compile_definitions(ShaderReflect PRIVATE ${AGE_BUILD_CONFIG})

set_target_properties(ShaderReflect PROPERTIES
	DEBUG_POSTFIX "d")
