cmake_minimum_required(VERSION 3.5)

set(project tests-systemid)
project(
	${project}
        DESCRIPTION "Test of systemid generation"
	LANGUAGES CXX
)

message("Configuring ${project}...")

include_directories(${INCLUDE_DIRS})

add_executable(${project} main.cpp)

set_target_properties(${project} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries(${project} ThorQLib ${LIBRARIES})

