# Preserve the compiler's raw /showIncludes prefix. CMake's default OEM decoding
# can corrupt UTF-8 diagnostics from a localized MSVC installation.
if(MSVC AND CMAKE_GENERATOR MATCHES "Ninja")
    set(_includesProbe "${CMAKE_BINARY_DIR}/CMakeFiles/AlienShowIncludes.cpp")
    file(WRITE "${_includesProbe}" "#include <stddef.h>\n")
    execute_process(
        COMMAND "${CMAKE_CXX_COMPILER}" /nologo /showIncludes /c "${_includesProbe}"
            "/Fo${CMAKE_BINARY_DIR}/CMakeFiles/AlienShowIncludes.obj"
        RESULT_VARIABLE _includesResult
        OUTPUT_VARIABLE _includesOutput
        ERROR_VARIABLE _includesError
        ENCODING NONE
    )
    string(REGEX MATCH "[^\r\n]*[A-Za-z]:[/\\\\][^\r\n]*" _includesLine "${_includesOutput}")
    string(REGEX REPLACE "[A-Za-z]:[/\\\\].*$" "" _includesPrefix "${_includesLine}")
    if(NOT _includesResult EQUAL 0 OR NOT _includesPrefix)
        message(FATAL_ERROR "Cannot determine the MSVC header dependency prefix: ${_includesError}")
    endif()
    if(EXISTS "${CMAKE_BINARY_DIR}/.ninja_deps" AND
       (NOT DEFINED ALIEN_MSVC_INCLUDE_PREFIX OR NOT ALIEN_MSVC_INCLUDE_PREFIX STREQUAL _includesPrefix))
        message(FATAL_ERROR
            "MSVC header dependency encoding changed. Recreate this build tree to discard stale objects. "
            "Agents must use build-agent and must not modify IDE-owned build trees.")
    endif()
    set(ALIEN_MSVC_INCLUDE_PREFIX "${_includesPrefix}" CACHE INTERNAL "Verified raw MSVC include prefix" FORCE)
    set(CMAKE_CL_SHOWINCLUDES_PREFIX "${_includesPrefix}")
endif()
