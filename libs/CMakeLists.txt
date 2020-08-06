cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
	${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h)
	# Set OS specific arguments
	if(LINUX)
		set(BUILD_OS "linux")
		set(BUILD_COMPILER "gcc")
	elseif(WINDOWS)
		set(BUILD_OS "windows")
		set(BUILD_COMPILER "msvc")
	endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(BUILD_ARCH "x86_64")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(BUILD_ARCH "i386")
	endif()

	# Set python initial arguments
	set(PYTHON_ARGS
		"${CMAKE_CURRENT_SOURCE_DIR}/botan/configure.py"
		"--os=${BUILD_OS}"
		"--cc=${BUILD_COMPILER}"
		"--cpu=${BUILD_ARCH}"
		"--amalgamation"
		"--disable-shared"
		"--link-method=copy"
		"--disable-modules=pkcs11"
		"--without-documentation"
	)

	# Crossing fingers....
	execute_process(
		COMMAND "python" ${PYTHON_ARGS}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		RESULT_VARIABLE py_result
		OUTPUT_VARIABLE py_output
	)

	# Print the output
	message("BUILD OUTPUT:\n${py_output}")

	# If it returned something else than 0 it probably didnt succeed
	if (NOT ${py_result} EQUAL 0)
		message(FATAL_ERROR "Failed to build Botan")
	endif()
endif()

set(${project}_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/botan_all.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/crypto.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/systemid.cpp
)

set(${project}_HEADERS
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include/enet.h

	${CMAKE_CURRENT_SOURCE_DIR}/log.h
	${CMAKE_CURRENT_SOURCE_DIR}/crypto.h
	${CMAKE_CURRENT_SOURCE_DIR}/systemid.h

	${CMAKE_CURRENT_SOURCE_DIR}/enums.h
	${CMAKE_CURRENT_SOURCE_DIR}/constants.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_auth.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_event.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_collar.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_crypto.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_version.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_command.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_heartbeat.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_command_ack.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_notification.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_payload_announcement.h
)

if (WINDOWS)
	list(APPEND ${project}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/systemid_windows.cpp)
elseif(LINUX)
	list(APPEND ${project}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/systemid_linux.cpp)
endif()

add_library(${project} STATIC
	${${project}_SOURCES}
	${${project}_HEADERS}
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Disable "deprecated header" messages
target_compile_definitions(${project} PRIVATE BOTAN_NO_DEPRECATED_WARNINGS)

if(WINDOWS)
	target_link_libraries(${project} Crypt32 winmm ws2_32 Iphlpapi)
endif()

set(LIB_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include

	CACHE PATH "List of library directories for header includes"
)
