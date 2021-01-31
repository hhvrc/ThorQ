cmake_minimum_required(VERSION 3.5)

set(project Tests)
project(
	${project}
        DESCRIPTION "A collection of tests"
	LANGUAGES CXX
)

message("Configuring ${project}...")

add_subdirectory(cryptography)