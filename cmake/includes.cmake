# A library's includes, read off its sources (src/<library>/ and private/<library>/ under its folder): each is the library's own, the standard library's or one a rule allows, and no public header includes a private one, which a consumer cannot reach. Included, this file defines vkm_test_includes(<name> <library> [ALLOW <rule>...]), which adds that check of the target <library> as the ctest <name>, labeled with the prefix before its colon. A rule is "<headers>", which any of its files may include, or "<headers> in <files>", which those alone may, each a regular expression matched whole against an include's path or a file's name. When ctest runs, the script the function wrote includes this file again.

if(NOT DEFINED VKM_INCLUDES_LIBRARY)
    function(vkm_test_includes name library)
        cmake_parse_arguments(PARSE_ARGV 2 _arg "" "" "ALLOW")
        if(_arg_UNPARSED_ARGUMENTS)
            message(FATAL_ERROR "vkm_test_includes(\"${name}\" ${library}) takes ALLOW, not: ${_arg_UNPARSED_ARGUMENTS}")
        endif()
        get_target_property(_library_dir ${library} SOURCE_DIR)
        # A script of its own, as a rule holds the '|' and ';' a -D argument would split on.
        string(SHA1 _script "${name}")
        string(SUBSTRING ${_script} 0 8 _script)
        set(_script ${CMAKE_CURRENT_BINARY_DIR}/includes/${_script}.cmake)
        file(WRITE ${_script}
            "set(VKM_INCLUDES_LIBRARY ${library})\n"
            "set(VKM_INCLUDES_DIR [==[${_library_dir}]==])\n"
            "set(VKM_INCLUDES_ALLOW [==[${_arg_ALLOW}]==])\n"
            "include([==[${CMAKE_CURRENT_FUNCTION_LIST_FILE}]==])\n"
        )
        add_test(NAME ${name} COMMAND ${CMAKE_COMMAND} -P ${_script})
        string(REGEX MATCH "^[^:]+" _label "${name}")
        set_tests_properties(${name} PROPERTIES LABELS ${_label})
    endfunction()
    return()
endif()

set(_src "${VKM_INCLUDES_DIR}/src")
set(_private "${VKM_INCLUDES_DIR}/private")
file(GLOB_RECURSE _public "${_src}/${VKM_INCLUDES_LIBRARY}/*.h")
file(GLOB_RECURSE _files
    "${_src}/${VKM_INCLUDES_LIBRARY}/*.h"
    "${_src}/${VKM_INCLUDES_LIBRARY}/*.cpp"
    "${_private}/${VKM_INCLUDES_LIBRARY}/*.h"
)

set(_problems "")
foreach(_file IN LISTS _files)
    get_filename_component(_name "${_file}" NAME)
    list(FIND _public "${_file}" _at)
    file(STRINGS "${_file}" _includes REGEX "^[ \t]*#[ \t]*include")
    foreach(_line IN LISTS _includes)
        string(REGEX MATCH "[<\"]([^>\"]+)[>\"]" _ "${_line}")
        set(_header "${CMAKE_MATCH_1}")
        set(_own FALSE)
        if(_header MATCHES "^${VKM_INCLUDES_LIBRARY}/")
            set(_own TRUE)
        endif()
        if(_own AND EXISTS "${_private}/${_header}")
            if(NOT _at EQUAL -1)
                string(APPEND _problems "  ${_file}: ${_line} (a public header, including a private one)\n")
            endif()
            continue()
        endif()
        if((_own AND EXISTS "${_src}/${_header}") OR _line MATCHES "<[a-z_]+>")
            continue()
        endif()
        set(_allowed FALSE)
        set(_only "")
        foreach(_rule IN LISTS VKM_INCLUDES_ALLOW)
            set(_files_of_rule ".*")
            if(_rule MATCHES "^(.+) in (.+)$")
                set(_rule "${CMAKE_MATCH_1}")
                set(_files_of_rule "${CMAKE_MATCH_2}")
            endif()
            if(NOT _header MATCHES "^(${_rule})$")
                continue()
            elseif(_name MATCHES "^(${_files_of_rule})$")
                set(_allowed TRUE)
            else()
                list(APPEND _only "${_files_of_rule}")
            endif()
        endforeach()
        if(_allowed)
        elseif(_only)
            string(REPLACE ";" ", " _only "${_only}")
            string(APPEND _problems "  ${_file}: ${_line} (only ${_only} may include it)\n")
        else()
            string(APPEND _problems "  ${_file}: ${_line}\n")
        endif()
    endforeach()
endforeach()

if(_problems)
    message(FATAL_ERROR
        "Includes the library may not have (its own, the standard library's, or one its vkm_test_includes call "
        "allows; a public header no private one):\n${_problems}")
endif()
