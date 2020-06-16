cmake_minimum_required(VERSION 3.5)

set(project Server)
project(${project})
message("Configuring ${project}...")

add_executable(${project}
	enums.h
	crypto.h
	crypto.cpp
	instance.h
	instance.cpp
	peermap.h
	peermap.cpp
	main.cpp
	)

target_link_libraries(${project} ThorQLib)
