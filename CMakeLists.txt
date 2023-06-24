cmake_minimum_required(VERSION 3.15)
project(epollsocket)

set(CMAKE_CXX_STANDARD 11)

aux_source_directory(tech SRC_LISTS)

add_executable(epollsocket main.cpp ${SRC_LISTS})

if(APPLE)
    target_include_directories(epollsocket PRIVATE includes)
endif(APPLE)
