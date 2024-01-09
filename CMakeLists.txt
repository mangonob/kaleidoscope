cmake_minimum_required(VERSION 3.10)

project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCE_FILES ${SRC}/*.cpp)

find_package(LLVM REQUIRED CONFIG)

# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs support core irreader analysis native)
message(STATUS "Components mapped to libnames: ${llvm_libs}")

add_executable(kalec ${SOURCE_FILES}
  ${SRC}/parser.tab.cpp
  ${SRC}/lexer.yy.cc
)

add_library(runtime STATIC lib/runtime.c)

install(TARGETS kalec DESTINATION bin)

target_include_directories(kalec PUBLIC ${INCLUDE})

set(asm_parser_ignore NVPTX XCore)
foreach(target ${LLVM_TARGETS_TO_BUILD})
  list(APPEND targets "LLVM${target}CodeGen")
  if (NOT ${target} IN_LIST asm_parser_ignore)
    list(APPEND targets "LLVM${target}AsmParser")
  endif()
endforeach()

message(STATUS "Targets: ${targets}")

# Link against LLVM libraries
target_link_libraries(kalec PRIVATE
  ${llvm_libs}
  ${targets}
  argparse
)

exec_program(brew ARGS --prefix bison OUTPUT_VARIABLE BISON)

add_custom_command(
  OUTPUT ${SRC}/parser.tab.cpp ${INCLUDE}/parser.tab.hpp ${INCLUDE}/location.hh
  COMMAND ${BISON}/bin/bison --header=${INCLUDE}/parser.tab.hpp -o ${SRC}/parser.tab.cpp ${SRC}/parser.yy
  DEPENDS ${SRC}/parser.yy
)

add_custom_command(
  OUTPUT ${SRC}/lexer.yy.cc ${INCLUDE}/lexer.yy.hh
  COMMAND flex --header-file=${INCLUDE}/lexer.yy.hh -o ${SRC}/lexer.yy.cc ${SRC}/lexer.x
  DEPENDS ${SRC}/lexer.x
)
