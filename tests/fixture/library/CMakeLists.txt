# A library in a vkm library's shape, for vkm_test_consumer and vkm_test_flags to pass: it stops without the target it requires, and its asserts are on, its sources compiling with -UNDEBUG whatever the build type.
cmake_minimum_required(VERSION 3.25)
project(vkm_fixture LANGUAGES CXX)

if(NOT TARGET vkm_fixture_needed)
    message(FATAL_ERROR "vkm_fixture requires the target 'vkm_fixture_needed', which the consuming project must add first.")
endif()

add_library(vkm_fixture STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/vkm_fixture/half.cpp)
target_link_libraries(vkm_fixture PUBLIC vkm_fixture_needed)
target_compile_options(vkm_fixture PRIVATE -UNDEBUG)
