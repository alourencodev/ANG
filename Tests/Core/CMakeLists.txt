
# Core CMakeFile

cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(Test_Core)
message(" === Tests: ${PROJECT_NAME}")

file(GLOB_RECURSE TEST_CORE_SOURCE "*.h" "*.hpp" "*.cpp")

add_executable(Test_Core ${TEST_CORE_SOURCE})

target_include_directories(Test_Core PUBLIC "${CMAKE_SOURCE_DIR}/Core" "${PROJECT_SOURCE_DIR}/Vendor")
target_link_libraries(Test_Core PRIVATE Core)
target_compile_options(Test_Core PRIVATE ${AGE_COMPILE_OPTIONS})
target_compile_definitions(Test_Core PRIVATE ${AGE_BUILD_CONFIG} ${AGE_TEST_FLAGS})

set_target_properties(Test_Core PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${AGE_TEST_RUNTIME_OUTPUT_DEBUG})
set_target_properties(Test_Core PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${AGE_TEST_RUNTIME_OUTPUT_RELEASE})
