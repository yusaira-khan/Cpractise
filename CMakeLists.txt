cmake_minimum_required(VERSION 3.2)
project(tiny_shell)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -std=c99")
set(SOURCE_FILES oscode.c )
add_executable(tiny_shell ${SOURCE_FILES})