cmake_minimum_required(VERSION 3.23)
project(TryDrawing)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

set(CMAKE_CXX_STANDARD 14)

add_executable(TryDrawing main.cpp)

INCLUDE(FindPkgConfig)

if(MACOSX)
    set(SDL2_LIB "/Library/Frameworks/SDL2.framework/SDL2")
    target_include_directories(TryDrawing PRIVATE "/Library/Frameworks/SDL2.framework/Headers")

    target_link_libraries(TryDrawing ${SDL2_LIB})
endif()