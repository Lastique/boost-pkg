cmake_minimum_required (VERSION 2.6)

set(TARGET dep_tree)

add_definitions(-DDEP_TREE_SRC_BUILD=1)

include_directories(../include)

add_library(${TARGET} STATIC
	../include/dep_tree.hpp
	../include/cxx_parser.hpp
	../include/filesystem_scanner.hpp
	../include/path_iterator.hpp
	../include/json.hpp
	../src/dep_tree.cpp
	../src/cxx_parser.cpp
	../src/filesystem_scanner.cpp
	../src/json.cpp
)
