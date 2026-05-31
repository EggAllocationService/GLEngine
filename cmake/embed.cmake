
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(util_embed "cmake/embed.cpp")



function(create_resource_library)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES)

    # Parse the arguments into variables
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${oneValueArgs}" "${multiValueArgs}")

    list(TRANSFORM arg_SOURCES PREPEND "${PROJECT_SOURCE_DIR}/")

    message("Adding target ${arg_NAME}")

    add_custom_command(
        OUTPUT ${arg_NAME}.c
               ${arg_NAME}.h
               COMMAND util_embed ${arg_NAME} ${arg_SOURCES}
               DEPENDS util_embed ${arg_SOURCES}
    )

    add_library(${arg_NAME} STATIC ${arg_NAME}.c ${arg_NAME}.h)
endfunction()