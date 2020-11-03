cmake_minimum_required(VERSION 3.5)

set(project Server)
project(
	${project}
	DESCRIPTION "Server to connect users"
	LANGUAGES CXX
)

message("Configuring ${project}...")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lsql)

set(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "Set to ON to build examples" FORCE)
set(CXXOPTS_BUILD_TESTS    OFF CACHE BOOL "Set to ON to build tests"    FORCE)
set(CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "Generate the install target" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cxxopts)

if (WIN32)
    include_directories(
	"C:\\Program Files (x86)\\TBB\\include"
    )
    link_directories(
	"C:\\Program Files (x86)\\TBB\\lib"
    )
endif()

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memorymanager.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/uuid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/session.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/relationship.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/messagedispatcher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memorymanager.h

    ${CMAKE_CURRENT_SOURCE_DIR}/uuid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/statistics.h

    ${CMAKE_CURRENT_SOURCE_DIR}/account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/session.h

    ${CMAKE_CURRENT_SOURCE_DIR}/authtoken.h
    ${CMAKE_CURRENT_SOURCE_DIR}/relationship.h

    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_server.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrentqueue
    ${CMAKE_CURRENT_SOURCE_DIR}/lsql/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include
)

add_executable(${project}
	main.cpp
	${${project}_SOURCES}
	${${project}_HEADERS}
)

if(WIN32)
    set(EXTRA_LIBS Rpcrt4)
else()
    set(EXTRA_LIBS uuid)
endif()

target_link_libraries(${project}
    ${EXTRA_LIBS}
    ThorQLib
    cxxopts
    lsql
    tbb
)
