# Runs a fixture binary once and checks how it ended (cmake -P):
#
#   -DBINARY=<file>          the fixture
#   -DTEST=<name>            what it is given: a test's full name, or --list
#   -DSTATUS=<code>          what it must exit with
#   -DSAYS=<a|b>             what its output must hold, in this order
#   -DNOT_SAYS=<a|b>         what its output must not hold
#   -DKEPT=<file>            a file the run must leave behind
#   -DREMOVED=<folder>       a folder it must not leave behind
#   -DENVIRONMENT=<k=v|k=>   what the run's environment sets, or with nothing after the '=' takes away

string(REPLACE "|" ";" _environment "${ENVIRONMENT}")
foreach(_setting IN LISTS _environment)
    string(FIND "${_setting}" "=" _at)
    string(SUBSTRING "${_setting}" 0 ${_at} _key)
    math(EXPR _at "${_at} + 1")
    string(SUBSTRING "${_setting}" ${_at} -1 _value)
    if(_value STREQUAL "")
        unset(ENV{${_key}})
    else()
        set(ENV{${_key}} "${_value}")
    endif()
endforeach()

# A file left by an earlier run is not this run's.
if(KEPT)
    file(REMOVE "${KEPT}")
endif()

execute_process(
    COMMAND "${BINARY}" "${TEST}"
    RESULT_VARIABLE _status
    OUTPUT_VARIABLE _output
    ERROR_VARIABLE  _output
    TIMEOUT 60
)
message("${BINARY} \"${TEST}\" exited ${_status}, saying:\n${_output}")

set(_problems "")
if(NOT _status STREQUAL STATUS)
    string(APPEND _problems "  it exited ${_status}, not ${STATUS}\n")
endif()

string(REPLACE "|" ";" _says "${SAYS}")
set(_rest "${_output}")
foreach(_text IN LISTS _says)
    string(FIND "${_rest}" "${_text}" _at)
    if(_at EQUAL -1)
        string(APPEND _problems "  it did not say '${_text}' where it should\n")
        continue()
    endif()
    string(LENGTH "${_text}" _length)
    math(EXPR _at "${_at} + ${_length}")
    string(SUBSTRING "${_rest}" ${_at} -1 _rest)
endforeach()

string(REPLACE "|" ";" _not_says "${NOT_SAYS}")
foreach(_text IN LISTS _not_says)
    string(FIND "${_output}" "${_text}" _at)
    if(NOT _at EQUAL -1)
        string(APPEND _problems "  it said '${_text}'\n")
    endif()
endforeach()

if(KEPT AND NOT EXISTS "${KEPT}")
    string(APPEND _problems "  it left no ${KEPT}\n")
endif()
if(REMOVED AND EXISTS "${REMOVED}")
    string(APPEND _problems "  it left ${REMOVED}\n")
endif()

if(_problems)
    message(FATAL_ERROR "Not as it should end:\n${_problems}")
endif()
