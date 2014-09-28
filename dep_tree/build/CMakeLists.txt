cmake_minimum_required (VERSION 2.6)

set(TARGET dep_tree)

add_definitions(-DDEP_TREE_SRC_BUILD=1)

include_directories(../include)

add_library(${TARGET} STATIC
	../include/dep_tree.hpp
	../src/dep_tree.cpp
)
