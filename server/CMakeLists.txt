cmake_minimum_required(VERSION 3.5)

set(project Server)
project(
	${project}
	DESCRIPTION "Server to connect users"
	LANGUAGES CXX
)

message("Configuring ${project}...")

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hasher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memorymanager.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/session.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/relationship.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/eventhandlers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hasher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memorymanager.h

    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/statistics.h

    ${CMAKE_CURRENT_SOURCE_DIR}/account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/session.h

    ${CMAKE_CURRENT_SOURCE_DIR}/eventhandlers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.h

    ${CMAKE_CURRENT_SOURCE_DIR}/singletons.h

    ${CMAKE_CURRENT_SOURCE_DIR}/authtoken.h
    ${CMAKE_CURRENT_SOURCE_DIR}/relationship.h

    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_server.h

    ${CMAKE_CURRENT_SOURCE_DIR}/concurrentqueue/concurrentqueue.h
)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrentqueue
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
