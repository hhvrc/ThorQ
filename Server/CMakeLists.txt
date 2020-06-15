cmake_minimum_required(VERSION 3.5)

set(project Server)

project(${project} LANGUAGES CXX)

if(LINUX)
	link_directories(/usr/local/lib/)
	include_directories(/usr/include/)
	include_directories(/usr/local/include/botan-2/)

	find_library(BOTAN2 NAMES botan2 botan-2 PATHS /usr/lib/ /usr/local/lib/)
elseif(WINDOWS)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/windows)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/windows/include)

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
elseif(WINDOWS)
	target_link_libraries(${project} botan)
endif()
