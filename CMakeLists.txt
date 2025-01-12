cmake_minimum_required(VERSION 3.22)
project(snake C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_subdirectory(libargs)
add_subdirectory(libsnake)
add_subdirectory(libtermbox2)

add_subdirectory(client)
add_subdirectory(server)
