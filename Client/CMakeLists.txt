cmake_minimum_required(VERSION 3.5)

set(project Client)
project(${project})
message("Configuring ${project}...")

add_executable(${project}
	enums.h
	crypto.h
	crypto.cpp
	main.cpp
	)

target_link_libraries(${project} ThorQLib)
