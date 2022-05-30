
# AGE CMakeFile
cmake_minimum_required(VERSION 3.22)

project(AGE)
message(" === Project: ${PROJECT_NAME}")

file(GLOB AGE_SOURCE "*.h" "*.hpp" "*.cpp")

add_library(AGE MODULE ${AGE_SOURCE})
target_include_directories(AGE PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(AGE PUBLIC Core)
