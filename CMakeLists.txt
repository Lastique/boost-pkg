cmake_minimum_required (VERSION 2.6)
project (boost-pkg)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

if(NOT "$ENV{BOOST_ROOT}" STREQUAL "")
	# message(STATUS "Setting BOOST_ROOT to $ENV{BOOST_ROOT}")
	set(BOOST_ROOT "$ENV{XCODER_BOOST_ROOT}")
endif()
if(NOT "$ENV{BOOST_INCLUDEDIR}" STREQUAL "")
	# message(STATUS "Setting BOOST_INCLUDEDIR to $ENV{BOOST_INCLUDEDIR}")
	set(BOOST_INCLUDEDIR "$ENV{BOOST_INCLUDEDIR}")
endif()
if(NOT "$ENV{BOOST_LIBRARYDIR}" STREQUAL "")
	# message(STATUS "Setting BOOST_LIBRARYDIR to $ENV{BOOST_LIBRARYDIR}")
	set(BOOST_LIBRARYDIR "$ENV{BOOST_LIBRARYDIR}")
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.55.0 REQUIRED)

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Could not find Boost library")
endif()

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})
#add_definitions(-DBOOST_ALL_DYN_LINK=1)

add_subdirectory(dep_tree/build)
