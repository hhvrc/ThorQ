cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
        ${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# ------ Get OpenSSL ------
find_package(OpenSSL REQUIRED)
set(${project}_LIBS
    OpenSSL::Crypto
    OpenSSL::SSL
)
if(WIN32)
    list(APPEND ${project}_LIBS Crypt32 winmm ws2_32 Iphlpapi)
endif()
# -------------------------

# ------- Get Botan -------
set(PYTHON_ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/botan/configure.py"
    "--os=${BUILD_OSL}"
    "--cc=${BUILD_COMPILER}"
    "--cpu=x86_${VPTR_BITS}"
    "--amalgamation"
    "--disable-shared"
    "--link-method=copy"
    "--minimized-build"
    "--enable-modules=auto_rng,bcrypt"
    "--without-documentation"
)
message("Executing: ${PYTHON_ARGS}")
execute_process(
    COMMAND "python" ${PYTHON_ARGS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RESULT_VARIABLE py_result
    OUTPUT_VARIABLE py_output
)
if (NOT ${py_result} EQUAL 0)
    message("BUILD OUTPUT:\n${py_output}")
    message(FATAL_ERROR "Failed to build Botan")
endif()

# Stop botan from fucking up
string(APPEND CMAKE_CXX_FLAGS " -DBOTAN_IS_BEING_BUILT -DBOTAN_NO_DEPRECATED_WARNINGS")

# Stop botan from fucking up (on windows)
if(WIN32)
    string(APPEND CMAKE_CXX_FLAGS " -bigobj")
endif()
# -------------------------

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hashing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid_${BUILD_OSL}.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h
    ${CMAKE_CURRENT_SOURCE_DIR}/enet/include/enet.h

    ${CMAKE_CURRENT_SOURCE_DIR}/crypto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hashing.h
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization.h

    ${CMAKE_CURRENT_SOURCE_DIR}/enums.h
    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_global.h
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_heartbeat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_crypto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_systemid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_relation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_moderation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_announcement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_collar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_ack.h
)

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(${project} STATIC
    ${${project}_SOURCES}
    ${${project}_HEADERS}
)

# Link libraries
target_link_libraries(${project}
    ${${project}_LIBS}
    ${Qt5Core_LIBRARIES}
)

set(GLOB_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include
	${CMAKE_CURRENT_BINARY_DIR}

	CACHE PATH "List of library directories for header includes"
)
