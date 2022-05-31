
# AGE CMakeFile
cmake_minimum_required(VERSION 3.22)

project(AGE)
message(" === Project: ${PROJECT_NAME}")

file(GLOB_RECURSE AGE_SOURCE "*.h" "*.hpp" "*.cpp")

add_library(AGE MODULE ${AGE_SOURCE})

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

target_include_directories(AGE PUBLIC ${CMAKE_SOURCE_DIR} ${Vulkan_INCLUDE_DIRS} ${GLFW3_INCLUDE_DIR})
target_link_libraries(AGE PUBLIC Core Vulkan::Vulkan ${GLFW3_LIBRARY})
