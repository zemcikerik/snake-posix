cmake_minimum_required(VERSION 3.29)
project(snake C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(libargs)
add_subdirectory(libsnake)
add_subdirectory(libtermbox2)

add_subdirectory(client)
add_subdirectory(server)
