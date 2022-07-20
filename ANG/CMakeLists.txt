
# ANG CMakeFile
cmake_minimum_required(VERSION 3.22)

project(ANG)
message(" === Project: ${PROJECT_NAME}")

file(GLOB_RECURSE ANG_SOURCE "*.h" "*.hpp" "*.cpp")

add_executable(ANG ${ANG_SOURCE})

target_include_directories(ANG PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(ANG PRIVATE Core AGE)
target_compile_options(ANG PRIVATE ${AGE_COMPILE_OPTIONS})
target_compile_definitions(ANG PRIVATE ${AGE_BUILD_CONFIG})

