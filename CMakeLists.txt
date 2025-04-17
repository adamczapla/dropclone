cmake_minimum_required(VERSION 3.30)

project(dropclone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(src bin)

option(ENABLE_TESTS "Build tests" ON)

if(ENABLE_TESTS)
  add_subdirectory(test)
endif()