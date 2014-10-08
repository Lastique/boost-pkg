cmake_minimum_required (VERSION 2.6)

set(TARGET boost-dep)

include_directories(${PROJECT_SOURCE_DIR}/dep_tree/include)

add_executable(${TARGET}
	../src/main.cpp
)

target_link_libraries(${TARGET}
	dep_tree
	boost_program_options
	boost_filesystem
	boost_system
)
