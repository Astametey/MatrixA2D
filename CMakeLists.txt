cmake_minimum_required(VERSION 3.26.1 FATAL_ERROR)
project(MatrixA2D)
 
set(CMAKE_CXX_STANDARD 20)

# 1. Вариант с ручным указанием путей (надежнее)
set(SFML_ROOT "D:/SFML")  # Укажите ваш реальный путь
set(SFML_INCLUDE_DIR "${SFML_ROOT}/include") 
set(SFML_LIBRARY_DIR "${SFML_ROOT}/lib")  
 
# Ищем библиотеки вручную 
find_path(SFML_INCLUDE_DIR "SFML/Graphics.hpp" PATHS ${SFML_INCLUDE_DIR})  
find_library(SFML_GRAPHICS_LIBRARY "sfml-graphics" PATHS ${SFML_LIBRARY_DIR}) 
find_library(SFML_WINDOW_LIBRARY "sfml-window" PATHS ${SFML_LIBRARY_DIR})  
find_library(SFML_SYSTEM_LIBRARY "sfml-system" PATHS ${SFML_LIBRARY_DIR}) 
find_library(SFML_AUDIO_LIBRARY "sfml-audio" PATHS ${SFML_LIBRARY_DIR})

if(NOT SFML_INCLUDE_DIR OR NOT SFML_GRAPHICS_LIBRARY)
    message(FATAL_ERROR "SFML not found. Check SFML_ROOT path")
endif()

# Настройки компилятора
if(MSVC)
    add_compile_options(/W4)  # Оставляем только уровень предупреждений
else()
    add_compile_options(-Wall -Wextra)  # Без -Werror
endif()
 
# Структура проекта  
set(ENGINE_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine) 
set(GAME_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/game) 

 
# Собираем файлы движка  
file(GLOB_RECURSE ENGINE_SOURCES 
    ${ENGINE_SRC_DIR}/core/*.cpp 
	${ENGINE_SRC_DIR}/core/*.h
    ${ENGINE_SRC_DIR}/entities/*.cpp 
	${ENGINE_SRC_DIR}/entities/*.h
    ${ENGINE_SRC_DIR}/world/*.cpp
	${ENGINE_SRC_DIR}/world/*.h
	${ENGINE_SRC_DIR}/resources/*.cpp
	${ENGINE_SRC_DIR}/resources/*.h 
	${ENGINE_SRC_DIR}/tinyxml/*.cpp
)

# Создаем библиотеку Engine
add_library(Engine STATIC ${ENGINE_SOURCES})
target_include_directories(Engine PUBLIC
    ${ENGINE_SRC_DIR}
    ${SFML_INCLUDE_DIR}
)

# Создаем исполняемый файл Game
add_executable(Game
    ${GAME_SRC_DIR}/main.cpp
    ${GAME_SRC_DIR}/MatrixA2D.cpp
)

target_link_libraries(Game
    Engine
    ${SFML_GRAPHICS_LIBRARY}
    ${SFML_WINDOW_LIBRARY}
    ${SFML_SYSTEM_LIBRARY}
    ${SFML_AUDIO_LIBRARY}
)

# Копирование ресурсов
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources 
     DESTINATION ${CMAKE_BINARY_DIR}/
)

# Копирование DLL файлов SFML
if(WIN32)
    # Для Debug-сборки
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        file(GLOB SFML_DLLS 
            "${SFML_ROOT}/bin/sfml-graphics-d-2.dll"
            "${SFML_ROOT}/bin/sfml-window-d-2.dll"
            "${SFML_ROOT}/bin/sfml-system-d-2.dll"
            "${SFML_ROOT}/bin/sfml-audio-d-2.dll"
        )
    # Для Release-сборки
    else()
        file(GLOB SFML_DLLS 
            "${SFML_ROOT}/bin/sfml-graphics-2.dll"
            "${SFML_ROOT}/bin/sfml-window-2.dll"
            "${SFML_ROOT}/bin/sfml-system-2.dll"
            "${SFML_ROOT}/bin/sfml-audio-2.dll"
        )
    endif()
    
    file(COPY ${SFML_DLLS} DESTINATION ${CMAKE_BINARY_DIR}/)
endif()

# Копирование ресурсов в бинарную директорию
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/bin)
