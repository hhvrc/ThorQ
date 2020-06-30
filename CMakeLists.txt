cmake_minimum_required(VERSION 3.5)

project(
	ThorQ
	DESCRIPTION "Software to control 3rd party collars"
	)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
	set(OS_STR "Linux")
elseif(WIN32)
	set(WINDOWS TRUE)
	set(OS_STR "Windows")
endif()

# 64-bit, threading, and verbose
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -pthread -Wall -Wextra -pedantic")

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -O3 -g0")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
endif()

# Very verbose!
if (VERBOSE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --verbose")
endif()

add_subdirectory(libs)
include_directories(${LIB_INC_DIRS})
add_subdirectory(Client)
add_subdirectory(Server)

if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	message("Building for ${OS_STR}, with full optimization (Release build)")
else()
	message("Building for ${OS_STR}, with no optimization (Debug build)")
endif()
