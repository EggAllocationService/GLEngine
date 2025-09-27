cmake_minimum_required(VERSION 3.3)
project(GLEngine)

set(CMAKE_CXX_STANDARD 20)

add_library(glengine STATIC
        src/Engine.cpp
        src/Widget.cpp "src/Colors.cpp")

target_include_directories(glengine PUBLIC include)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(glengine "-framework GLUT")
    target_link_libraries(glengine "-framework OpenGL")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(glengine PUBLIC "C:\\Program Files\\freeglut\\include")
   
    target_link_libraries(glengine "C:\\Program Files\\freeglut\\lib\\x64\\freeglut.lib")
endif()

add_executable(test1 test/main.cpp)
target_link_libraries(test1 PRIVATE glengine)