cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
	${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

if (NOT EXISTS ${CMAKE_BINARY_DIR}/botan_all.h)
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
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
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

add_library(${project} STATIC
	${CMAKE_BINARY_DIR}/botan_all.cpp
	${CMAKE_BINARY_DIR}/botan_all.h
	${CMAKE_CURRENT_SOURCE_DIR}/enums.h
	${CMAKE_CURRENT_SOURCE_DIR}/constants.h
	${CMAKE_CURRENT_SOURCE_DIR}/crypto.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/crypto.h

	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_crypto.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_crypto.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_version.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_version.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_announcement.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_announcement.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_collar.h
	${CMAKE_CURRENT_SOURCE_DIR}/thorq_message_collar.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/enet/include/enet.h
)

set(LIB_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include
	${CMAKE_BINARY_DIR}

	CACHE PATH "List of library directories for header includes"
)

include_directories(${LIB_INC_DIRS})

if(WINDOWS)
	target_link_libraries(${project} Crypt32)
endif()
