cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
	${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# --- Botan amalgamation build ---
set(PYTHON_ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/botan/configure.py"
    "--os=${BUILD_OSL}"
    "--cc=${BUILD_COMPILER}"
    "--cpu=${BUILD_ARCH}"
    "--amalgamation"
    "--disable-shared"
    "--link-method=copy"
    "--minimized-build"
    "--enable-modules=auto_rng,chacha_rng,chacha,chacha20poly1305,chacha_avx2,salsa20,seed,rdseed,sha2_32,stream,keypair,pubkey,ecdh,x509,bcrypt,kdf2,pbkdf2"
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

set(${project}_SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid_${BUILD_OSL}.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h
    ${CMAKE_CURRENT_SOURCE_DIR}/enet/include/enet.h

    ${CMAKE_CURRENT_SOURCE_DIR}/crypto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid.h

    ${CMAKE_CURRENT_SOURCE_DIR}/enums.h
    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_auth.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_event.h
	#${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_regkey.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_collar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_crypto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_instance.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_version.h
    #${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_systemid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_authtoken.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_heartbeat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_notification.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_announcement.h
)

# Stop botan from fucking up
string(APPEND CMAKE_CXX_FLAGS " -DBOTAN_IS_BEING_BUILT -DBOTAN_NO_DEPRECATED_WARNINGS")

if(WIN32)
    list(APPEND ${project}_LIBS Crypt32 winmm ws2_32 Iphlpapi)

    # Stop botan from fucking up (on windows)
	string(APPEND CMAKE_CXX_FLAGS " -bigobj -DNOMINMAX")
endif()

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
