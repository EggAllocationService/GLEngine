
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(paktool "cmake/paktool.cpp")


function(build_pak)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES NAMES)

    # Parse the arguments into variables
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${oneValueArgs}" "${multiValueArgs}")

    list(TRANSFORM arg_SOURCES PREPEND "${PROJECT_SOURCE_DIR}/")

    message("Adding target ${arg_NAME}")

    add_custom_command(
            OUTPUT ${arg_NAME}.pak
            COMMAND paktool ${arg_NAME}.pak ${arg_SOURCES} ${arg_NAMES}
            DEPENDS paktool ${arg_SOURCES}
    )

    add_custom_target(
            ${arg_NAME}
            DEPENDS ${arg_NAME}.pak
    )
endfunction()