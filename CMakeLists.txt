cmake_minimum_required(VERSION 3.26.1 FATAL_ERROR)

set(PROJECT_NAME MatrixA2D)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

include_directories(D:/SFML/include)

link_directories(D:/SFML/lib)

add_executable(${PROJECT_NAME}
        src/main.cpp
        )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources 
	$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

target_link_libraries(${PROJECT_NAME} PRIVATE openal32 sfml-window sfml-audio sfml-system sfml-graphics)