cmake_minimum_required(VERSION 3.5)

set(project Server)
project(
	${project}
	DESCRIPTION "Server to connect users"
	LANGUAGES CXX
)

message("Configuring ${project}...")

set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/instance.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/instancemap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/authhandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/eventhandlers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.cpp
)
set(${project}_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/instance.h
	${CMAKE_CURRENT_SOURCE_DIR}/typedefs.h
	${CMAKE_CURRENT_SOURCE_DIR}/singletons.h
	${CMAKE_CURRENT_SOURCE_DIR}/instancemap.h
	${CMAKE_CURRENT_SOURCE_DIR}/authhandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/eventhandlers.h
	${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.h
)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${project}
	main.cpp
	${${project}_SOURCES}
	${${project}_HEADERS}
)

target_link_libraries(${project}
    ${Qt5Core_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ThorQLib
)
