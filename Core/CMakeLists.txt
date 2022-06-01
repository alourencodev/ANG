
# Core CMakeFile
cmake_minimum_required(VERSION 3.22)

project(Core)
message(" === Project: ${PROJECT_NAME}")

file(GLOB_RECURSE CORE_SOURCE "*.h" "*.hpp" "*.cpp")

add_library(Core STATIC ${CORE_SOURCE})
target_include_directories(Core PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_options(Core PRIVATE ${AGE_COMPILE_OPTIONS})
