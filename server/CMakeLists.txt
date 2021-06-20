cmake_minimum_required (VERSION 3.5)

set (project Server)
project (
    ${project}
    DESCRIPTION "Server to connect users"
    LANGUAGES CXX
)

message ("Configuring ${project}...")

# --------- lsql ----------
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/lsql)
# -------------------------

# -------- cxxopts --------
set (CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "Set to ON to build examples" FORCE)
set (CXXOPTS_BUILD_TESTS    OFF CACHE BOOL "Set to ON to build tests"    FORCE)
set (CXXOPTS_ENABLE_INSTALL OFF CACHE BOOL "Generate the install target" FORCE)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/cxxopts)
# -------------------------

set (${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver_connection.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/imagecache.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_announcement.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_error.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_friendrequest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_group.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_moderation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_p2p.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_systemid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_user.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_version.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/singletons.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/group.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/messagehandlingcontext.cpp
)
set (${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apiserver_connection.h
    #${CMAKE_CURRENT_SOURCE_DIR}/imagecache.h
    ${CMAKE_CURRENT_SOURCE_DIR}/database.h

    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_announcement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_crypto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_error.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_file.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_friendrequest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_group.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_message.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_moderation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_p2p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_systemid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_user.h
    ${CMAKE_CURRENT_SOURCE_DIR}/api_endpoints/endpoint_version.h

    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/singletons.h

    ${CMAKE_CURRENT_SOURCE_DIR}/account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/group.h

    ${CMAKE_CURRENT_SOURCE_DIR}/authtoken.h

    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_server.h

    ${CMAKE_CURRENT_SOURCE_DIR}/messagehandlingcontext.h
)

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lsql/include
    ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include
)

add_executable (${project}
	main.cpp
	${${project}_SOURCES}
	${${project}_HEADERS}
)

target_link_libraries (${project}
    ThorQLib
    cxxopts
    lsql
)
