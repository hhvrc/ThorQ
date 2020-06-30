cmake_minimum_required(VERSION 3.5)

set(project Server)
project(
	${project}
	DESCRIPTION "Server to connect users"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message("Configuring ${project}...")

add_executable(${project}
	instance.h
	instance.cpp
	peermap.h
	peermap.cpp
	main.cpp
)

target_link_libraries(${project} ThorQLib)
