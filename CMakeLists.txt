cmake_minimum_required(VERSION 3.26.1 FATAL_ERROR)

set(PROJECT_NAME MatrixA2D)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
        src/main.cpp
        )
