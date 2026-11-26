# A project that adds the library as its README says: the targets it requires first, then the library. Stand-ins for those targets, as the library asks only that they exist when it is added.
cmake_minimum_required(VERSION 3.25)
project(vkm_consumer LANGUAGES C CXX)

string(REPLACE "|" ";" VKM_CONSUMER_ADDS "${VKM_CONSUMER_ADDS}")
foreach(_target IN LISTS VKM_CONSUMER_ADDS)
    add_library(${_target} INTERFACE)
endforeach()

add_subdirectory(${VKM_LIBRARY_DIR} library)
