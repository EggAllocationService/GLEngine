cmake_minimum_required(VERSION 3.3...4.0)
project(GLEngine)

option(TEXT_RENDERING "Compile builtin text rendering library (requires harfbuzz)" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(wgpu-native)
include(embed)
include(paktool)
include(harfbuzz)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

provide_wgpu_native("29.0.0.0")

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
add_subdirectory(external/glfw)
add_subdirectory(external/glfw3webgpu)

add_library(glengine SHARED
        "src/Engine.cpp"
        "src/Colors.cpp"
        "src/MatrixStack.cpp"
        "src/GLMath.cpp"
        "src/input/MouseManager.cpp"
        "src/3d/Transform.cpp"
        "src/3d/ActorPrimitiveComponent.cpp"
        "src/3d/components/CameraComponent.cpp"
        "src/3d/Pawn.cpp"
        "src/3d/DefaultPawn.cpp"
        "src/input/InputManager.cpp"
        "src/3d/mesh/StaticMesh.cpp"
        "src/3d/mesh/StaticMeshComponent.cpp"
        "src/3d/Actor.cpp"
        "src/pipeline/RenderObjects.cpp"
        "src/3d/components/PointLightComponent.cpp"
        external/stb_image.cpp
        src/pipeline/gl/GLRenderer.cpp
        src/3d/texture/StaticTexture2D.cpp
        src/3d/components/DirectionalLightComponent.cpp
        src/pipeline/wgpu/WGPURenderer.cpp
        include/pipeline/wgpu/WGPURenderer.h
        src/pipeline/wgpu/pipeline/RenderPipeline.cpp
        src/pipeline/wgpu/GPUMesh.cpp
        src/3d/ActorComponent.cpp
        src/pipeline/wgpu/pipeline/Pipeline.cpp
        include/pipeline/RenderObject.h
        include/pipeline/wgpu/TypedGPUBuffer.h
        src/3d/mesh/InstancedStaticMeshComponent.cpp
        include/3d/mesh/InstancedStaticMeshComponent.h
        "src/3d/objects/InstancedDrawTracker.cpp"
        "include/3d/objects/InstancedDrawTracker.h"
        src/3d/objects/LightTracker.cpp
        include/3d/objects/LightTracker.h
        "src/pipeline/wgpu/TransferManager.cpp"
        "include/pipeline/wgpu/TransferManager.h"
        src/ResourceManager.cpp
        src/pipeline/wgpu/pipeline/ComputePipeline.cpp
        include/pipeline/wgpu/pipeline/ComputePipeline.h
        include/pipeline/wgpu/GPUPointer.h
        src/pipeline/wgpu/WrappedBuffer.cpp
        include/pipeline/wgpu/WrappedBuffer.h
        include/pipeline/wgpu/GPUTexture.h
        include/PakFile.h
)

create_resource_library(NAME Shaders SOURCES 
    src/pipeline/wgpu/shaders/BasicLit.wgsl
    src/pipeline/wgpu/shaders/BasicLitInstanced.wgsl
)

if (${TEXT_RENDERING})
    provide_harfbuzz(14.2.0)

    target_compile_definitions(glengine PUBLIC GLENGINE_TEXT_RENDERING)

    target_sources(glengine PRIVATE
            src/3d/text/Font.cpp
            src/3d/text/truetype.c
            src/3d/text/SlugTextComponent.cpp
    )

    create_resource_library(NAME TextRenderingResources SOURCES
        src/3d/text/trim.ttf
        src/3d/text/QuattrocentoSans.ttf
        src/3d/text/slug.wgsl
    )
    target_link_libraries(glengine harfbuzz TextRenderingResources)
endif()

target_include_directories(glengine PUBLIC include external/include)

include(GenerateExportHeader)
generate_export_header(glengine)
target_include_directories(glengine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_sources(
        glengine
        PUBLIC
            FILE_SET generated_headers
                TYPE HEADERS
                BASE_DIRS $<TARGET_PROPERTY:glengine,BINARY_DIR>
                FILES ${CMAKE_CURRENT_BINARY_DIR}/glengine_export.h
)

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    set_property(TARGET glengine
            PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    target_compile_options(glengine PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:-mavx>
            $<$<CXX_COMPILER_ID:Clang>:-mavx>
            $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX>
    )
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(glengine "-framework GLUT")
    target_link_libraries(glengine "-framework OpenGL")
    SET(CMAKE_CXX_FLAGS_DEBUG "-g")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # also narrowing conversion warnings for integers??? (double -> float)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4244")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(glengine "glut")
    target_link_libraries(glengine "OpenGL")
    target_link_libraries(glengine "GLU")
endif()

target_link_libraries(glengine glfw webgpu glfw3webgpu Shaders)

function(engine_copy_freeglut_if_needed TARGETNAME)
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            # x86 libraries
            set(FREEGLUT_LIB "C:\\Program Files\\freeglut\\lib\\freeglut.lib")
            set(FREEGLUT_DLL "C:\\Program Files\\freeglut\\bin\\freeglut.dll")
        else()
            # x64 libraries
            set(FREEGLUT_LIB "C:\\Program Files\\freeglut\\lib\\x64\\freeglut.lib")
            set(FREEGLUT_DLL "C:\\Program Files\\freeglut\\bin\\x64\\freeglut.dll")
        endif()
        add_custom_command(TARGET ${TARGETNAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREEGLUT_DLL} $<TARGET_FILE_DIR:${TARGETNAME}>)
    endif()
endfunction()

function(copy_assets TARGET_NAME SOURCE_DIR)
    if(NOT TARGET_NAME OR NOT SOURCE_DIR)
        message(FATAL_ERROR "copy_assets requires a target name and a directory path.")
    endif()

    get_filename_component(SOURCE_DIR "${SOURCE_DIR}" ABSOLUTE)

    add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SOURCE_DIR}"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>"
            COMMENT "Copying assets from ${SOURCE_DIR} to target ${TARGET_NAME}"
            VERBATIM
    )
endfunction()

if (${PROJECT_IS_TOP_LEVEL})
    build_pak(
            NAME assets
            SOURCES
                test3d/assets/cube-tex.obj
                test3d/assets/enterprise.obj
            NAMES
                /cube.obj
                /enterprise.obj
    )

    add_executable(test3d test3d/main.cpp
            test3d/SpinnyCube.cpp
            test3d/SpinnyCube.h
            test3d/Enterprise.h
            test3d/Enterprise.cpp
            test3d/Environment.cpp
            test3d/Environment.h
    )

    copy_assets(test3d test3d/assets)

    target_link_libraries(test3d PRIVATE glengine)
    add_dependencies(test3d assets)
endif()