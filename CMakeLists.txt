cmake_minimum_required(VERSION 3.3...4.0)
project(GLEngine)

set(CMAKE_CXX_STANDARD 20)

add_library(glengine STATIC
        "src/Engine.cpp"
        "src/Widget.cpp"
        "src/Colors.cpp" 
        "src/MatrixStack.cpp"
        "src/GLMath.cpp"
        "src/widgets/Button.cpp" 
        "src/widgets/PerfCounter.cpp"
        "src/input/MouseManager.cpp")

target_include_directories(glengine PUBLIC include)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(glengine "-framework GLUT")
    target_link_libraries(glengine "-framework OpenGL")
    SET(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(glengine PUBLIC "C:\\Program Files\\freeglut\\include")

    if (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # x86 libraries
        set(FREEGLUT_LIB "C:\\Program Files\\freeglut\\lib\\freeglut.lib")
        set(FREEGLUT_DLL "C:\\Program Files\\freeglut\\bin\\freeglut.dll")
    else()
        # x64 libraries
        set(FREEGLUT_LIB "C:\\Program Files\\freeglut\\lib\\x64\\freeglut.lib")
        set(FREEGLUT_DLL "C:\\Program Files\\freeglut\\bin\\x64\\freeglut.dll")
    endif()

    target_link_libraries(glengine ${FREEGLUT_LIB})

    add_custom_command(TARGET glengine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREEGLUT_DLL} $<TARGET_FILE_DIR:glengine>)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(glengine "glut")
    target_link_libraries(glengine "OpenGL")
endif()

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

if (${PROJECT_IS_TOP_LEVEL})
    add_executable(test1 test/main.cpp "test/RgbTriangle.cpp")
    target_link_libraries(test1 PRIVATE glengine)

    if (MSVC AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        set_target_properties(test1 PROPERTIES LINK_FLAGS "/PROFILE")
    endif()
endif()

# enables profiling tools in VS
if (MSVC AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set_target_properties(glengine PROPERTIES LINK_FLAGS "/PROFILE")
endif()