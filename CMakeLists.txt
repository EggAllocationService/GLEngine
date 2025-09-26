cmake_minimum_required(VERSION 4.0)
project(GLEngine)

set(CMAKE_CXX_STANDARD 20)

add_library(glengine SHARED
        src/Engine.cpp
        src/Widget.cpp)

target_include_directories(glengine PUBLIC include)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(glengine "-framework GLUT")
    target_link_libraries(glengine "-framework OpenGL")
endif()

add_executable(test1 test/main.cpp)
target_link_libraries(test1 PRIVATE glengine)