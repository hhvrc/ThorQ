cmake_minimum_required(VERSION 3.5)

set(project Client)
project(${project})

if(LINUX)
	link_directories(/usr/local/lib/)
	include_directories(/usr/include/)
	include_directories(/usr/local/include/botan-2/)
elseif(WINDOWS)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/windows)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/windows/include)
endif()

add_executable(${project}
	enet.h
	enums.h
	crypto.h
	crypto.cpp
	main.cpp
	)

if(LINUX)
	target_link_libraries(${project} botan-2)
elseif(WINDOWS)
	target_link_libraries(${project} botan)
endif()
