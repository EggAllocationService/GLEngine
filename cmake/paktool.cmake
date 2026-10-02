
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(meshoptimizer
    GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
    GIT_TAG v1.3
    GIT_SHALLOW TRUE
)
set(MESHOPT_INSTALL OFF)

FetchContent_MakeAvailable(meshoptimizer)

add_library(optimizer STATIC cmake/optimizer.cpp cmake/optimizer.h "cmake/tiny_obj_loader.h")
target_link_libraries(optimizer PRIVATE meshoptimizer zstd)

add_executable(paktool "cmake/paktool.cpp")
target_link_libraries(paktool optimizer)

add_executable(meshtool "cmake/opttool.cpp")
target_link_libraries(meshtool optimizer)

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