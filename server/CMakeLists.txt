cmake_minimum_required(VERSION 3.5)

set(project Server)
project(
	${project}
	DESCRIPTION "Server to connect users"
	LANGUAGES CXX
)

message("Configuring ${project}...")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cxxopts)

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memorymanager.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/session.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/relationship.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/query.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/column.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/transaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/connection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/internal/shell.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/internal/sqlite3.c
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memorymanager.h

    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/statistics.h

    ${CMAKE_CURRENT_SOURCE_DIR}/account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/session.h

    ${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.h

    ${CMAKE_CURRENT_SOURCE_DIR}/authtoken.h
    ${CMAKE_CURRENT_SOURCE_DIR}/relationship.h

    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_server.h

    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/type.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/value.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/query.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/column.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/transaction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/connection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/internal/sqlite3.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sqlite/internal/sqlite3ext.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrentqueue
    ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include
)

add_executable(${project}
	main.cpp
	${${project}_SOURCES}
	${${project}_HEADERS}
)

target_link_libraries(${project}
    ThorQLib
    cxxopts
)
