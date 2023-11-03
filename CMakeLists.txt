cmake_minimum_required(VERSION 3.10)

project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(kalec ${SOURCE_FILES})

install(TARGETS kalec DESTINATION bin)

target_include_directories(kalec PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
)
