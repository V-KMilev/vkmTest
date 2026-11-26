# Every #include under src/vkm_test is the library's own or the standard library's, or a system header in a file that reaches the system, so it depends on nothing of ours (cmake -P, -DSOURCE_DIR=<src>).

# The system headers each file that reaches the system may include: Windows' in the runner and the aligned blocks, POSIX's in the child process.
set(_system_main.cpp "windows.h")
set(_system_allocations.cpp "malloc.h")
set(_system_scratch.cpp "sys/resource.h|sys/wait.h|unistd.h")

file(GLOB_RECURSE _files
    "${SOURCE_DIR}/vkm_test/*.h"
    "${SOURCE_DIR}/vkm_test/*.cpp"
)
set(_problems "")
foreach(_file IN LISTS _files)
    get_filename_component(_name "${_file}" NAME)
    file(STRINGS "${_file}" _includes REGEX "^[ \t]*#[ \t]*include")
    foreach(_line IN LISTS _includes)
        string(REGEX MATCH "[<\"]([^>\"]+)[>\"]" _ "${_line}")
        set(_header "${CMAKE_MATCH_1}")
        if(_header MATCHES "^vkm_test/" AND EXISTS "${SOURCE_DIR}/${_header}")
        elseif(_line MATCHES "<[a-z_]+>")
        elseif(DEFINED "_system_${_name}" AND _header MATCHES "^(${_system_${_name}})$")
        else()
            string(APPEND _problems "  ${_file}: ${_line}\n")
        endif()
    endforeach()
endforeach()

if(_problems)
    message(FATAL_ERROR "Includes the library may not have:\n${_problems}")
endif()
