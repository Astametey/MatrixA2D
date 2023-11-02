cmake_minimum_required(VERSION 3.26.1 FATAL_ERROR)

set(PROJECT_NAME MatrixA2D)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME}
        src/main.cpp
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources 
	$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

