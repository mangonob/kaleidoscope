cmake_minimum_required(VERSION 3.10)

project(Kaleidoscope)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCE_FILES ${SRC}/*.cpp)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(kalec ${SOURCE_FILES}
  # ${SRC}/parser.tab.cpp
  ${SRC}/lexer.yy.cc
)

install(TARGETS kalec DESTINATION bin)

target_include_directories(kalec PUBLIC ${INCLUDE})

llvm_map_components_to_libnames(llvm_libs support core irreader)

# Link against LLVM libraries
target_link_libraries(kalec ${llvm_libs})

exec_program(brew ARGS --prefix bison OUTPUT_VARIABLE BISON)

# add_custom_command(
#   OUTPUT ${SRC}/parser.tab.cpp ${INCLUDE}/parser.tab.hpp ${INCLUDE}/location.hh
#   COMMAND ${BISON}/bin/bison --header=${INCLUDE}/parser.tab.hpp -o ${SRC}/parser.tab.cpp ${SRC}/parser.yy
#   DEPENDS ${SRC}/parser.yy
# )

add_custom_command(
  OUTPUT ${SRC}/lexer.yy.cc ${INCLUDE}/lexer.yy.hh
  COMMAND flex --header-file=${INCLUDE}/lexer.yy.hh -o ${SRC}/lexer.yy.cc ${SRC}/lexer.x
  DEPENDS ${SRC}/lexer.x
)
