cmake_minimum_required(VERSION 3.5)

set(project Client)
project(${project})
message("Configuring ${project}...")

add_executable(${project}
	gui.h
	gui.cpp
	main.cpp
	instance.h
	instance.cpp
)

target_link_libraries(${project} ThorQLib)
