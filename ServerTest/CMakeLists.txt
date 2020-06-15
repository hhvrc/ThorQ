cmake_minimum_required(VERSION 3.5)

set(project ServerTest)

project(${project} LANGUAGES CXX)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(LINUX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC -g")

	if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
		message("Building with no optimization (Debug build)")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --verbose")
	else()
		message("Building with full optimization (Release build)")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
	endif()

	link_directories(/usr/local/lib/)
	include_directories(/usr/include/)
	include_directories(/usr/local/include/botan-2/)

	find_library(BOTAN2 NAMES botan2 botan-2 PATHS /usr/lib/ /usr/local/lib/)
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")

	link_directories(${CMAKE_SOURCE_DIR}/external/windows)
	include_directories(${CMAKE_SOURCE_DIR}/external/windows/include)
	find_library(BOTAN NAMES botan)
endif()

add_executable(${project}
	enet.h
	enums.h
	crypto.h
	crypto.cpp
	instance.h
	instance.cpp
	peermap.h
	peermap.cpp
	main.cpp
	)

if(LINUX)
	target_link_libraries(${project} botan-2)
elseif(WIN32)
	target_link_libraries(${project} botan)
endif()