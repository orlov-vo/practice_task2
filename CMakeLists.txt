cmake_minimum_required(VERSION 2.8.4)
project(practice_task2)

set(SOURCE_FILES main.c)
set(CMAKE_C_FLAGS "-std=c11 ${CMAKE_C_FLAGS} -g")

add_executable(practice_task2 ${SOURCE_FILES})
