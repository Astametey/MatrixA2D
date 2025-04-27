cmake_minimum_required(VERSION 3.26.1 FATAL_ERROR)

set(PROJECT_NAME MatrixA2D)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

include_directories(D:/SFML/include)

link_directories(D:/SFML/lib)

add_executable(${PROJECT_NAME}
        src/main.cpp
		
		
		src/game.h
		src/level.h
		src/view.h
		src/entity.h
		src/enemies.h
		src/player.h
		
		src/pathfindingA.h
		
		src/tinyxml/tinystr.cpp
		src/tinyxml/tinyxml.cpp
		src/tinyxml/tinyxml2.cpp
		src/tinyxml/tinyxmlerror.cpp
		src/tinyxml/tinyxmlparser.cpp
		
		src/tinyxml/tinyxml.h
		src/tinyxml/tinyxml2.h
		src/tinyxml/tinystr.h
        )
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/src/resources 
	$<TARGET_FILE_DIR:${PROJECT_NAME}>/resources ../build/resources)
	
target_link_libraries(${PROJECT_NAME} PRIVATE openal32 sfml-window sfml-audio sfml-system sfml-graphics)