cmake_minimum_required(VERSION 3.10)

project(libavltree)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE HEADER_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)
source_group(TREE "${PROJECT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_FILES})
add_library(libavltree INTERFACE)
  
target_include_directories(libavltree INTERFACE
  "${PROJECT_SOURCE_DIR}/include"
)

enable_testing()
add_executable(testavltree ${PROJECT_SOURCE_DIR}/test/main.cpp ${PROJECT_SOURCE_DIR}/test/framework.hpp)
target_link_libraries(testavltree INTERFACE libavltree)
target_include_directories(testavltree PUBLIC
  "${PROJECT_SOURCE_DIR}/test"
  "${PROJECT_SOURCE_DIR}/include"
)
add_test(NAME testavltree COMMAND testavltree)
