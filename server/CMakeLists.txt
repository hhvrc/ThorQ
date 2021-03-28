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

# -- bad email providers --
message("bad email providers: ${BAD_EMAIL_PROVIDERS_FILE}")
file(COPY ${BAD_EMAIL_PROVIDERS_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bad-providers.txt)
# -------------------------

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver_connection.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/imagecache.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/singletons.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/group.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver_connection.h
    #${CMAKE_CURRENT_SOURCE_DIR}/imagecache.h
    ${CMAKE_CURRENT_SOURCE_DIR}/database.h

    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/singletons.h

    ${CMAKE_CURRENT_SOURCE_DIR}/account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/group.h

    ${CMAKE_CURRENT_SOURCE_DIR}/authtoken.h

    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_server.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lsql/include
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
    lsql
)
