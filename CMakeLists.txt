cmake_minimum_required(VERSION 3.25.0)

project(vkm_test VERSION 1.0.0 LANGUAGES CXX)

message("[VKM_TEST] Building vkm_test v${PROJECT_VERSION}")

file(GLOB_RECURSE _vkm_test_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# Static, with the runner's main: a suite's binary is its tests and this.
add_library(${PROJECT_NAME} STATIC ${_vkm_test_sources})

# The root above vkm_test/, so an include names the library: "vkm_test/check.h".
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Its warning set. Clang reads the doc blocks: a @param naming no parameter is an error.
set(_vkm_test_warnings
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wformat=2
    -Wnon-virtual-dtor
    -Woverloaded-virtual
)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    list(APPEND _vkm_test_warnings -Werror=documentation)
endif()
target_compile_options(${PROJECT_NAME} PRIVATE ${_vkm_test_warnings})

# What a suite's CMake calls, defined for whoever adds the library: vkm_discover_tests, vkm_test_consumer and vkm_test_flags.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/discover.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/consumer.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/flags.cmake)

# The suite (tests/): built alone, the library tests itself; a consumer asks for it.
option(VKM_TEST_BUILD_TESTS "Build the vkm_test test suite" ${PROJECT_IS_TOP_LEVEL})
if(VKM_TEST_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
