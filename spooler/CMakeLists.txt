cmake_minimum_required(VERSION 3.3)
project(spooler)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(S_SOURCE_FILES server.c info.h)
add_executable(server ${S_SOURCE_FILES})
target_link_libraries(server rt pthread)

set(C_SOURCE_FILES client.c info.h)
add_executable(client ${C_SOURCE_FILES})
target_link_libraries(client rt pthread)
