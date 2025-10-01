cmake_minimum_required(VERSION 3.3...4.0)
project(GLEngine)

set(CMAKE_CXX_STANDARD 20)

add_library(glengine STATIC
        "src/Engine.cpp"
        "src/Widget.cpp"
        "src/Colors.cpp" 
        "src/MatrixStack.cpp"
        src/GLMath.cpp
        include/GLMath.h 
        "src/widgets/Button.cpp" 
        "src/widgets/PerfCounter.cpp"
        src/private/MouseManager.cpp
        include/MouseManager.h)

target_include_directories(glengine PUBLIC include)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(glengine "-framework GLUT")
    target_link_libraries(glengine "-framework OpenGL")
    SET(CMAKE_CXX_FLAGS_DEBUG "-g -Og")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_include_directories(glengine PUBLIC "C:\\Program Files\\freeglut\\include")
   
    target_link_libraries(glengine "C:\\Program Files\\freeglut\\lib\\x64\\freeglut.lib")
endif()

add_executable(test1 test/main.cpp "test/RgbTriangle.cpp")
target_link_libraries(test1 PRIVATE glengine)