cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
	${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# ---Qt---
list(APPEND ${project}_LIBS ${Qt5_LIBS})
list(APPEND INCLUDE_DIRS ${Qt5_DIRS})
get_target_property (QT_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
macro(qt_add_resources)
   qt5_add_resources(${ARGN})
endmacro()
macro(qt_wrap_cpp)
        qt5_wrap_cpp(${ARGN})
endmacro()
macro(qt_wrap_ui)
        qt5_wrap_ui(${ARGN})
endmacro()
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

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


# ---Botan---
# Set python initial arguments
set(PYTHON_ARGS
	"${CMAKE_CURRENT_SOURCE_DIR}/botan/configure.py"
	"--os=${BUILD_OS}"
	"--cc=${BUILD_COMPILER}"
	"--cpu=${BUILD_ARCH}"
	"--amalgamation"
	"--disable-shared"
	"--link-method=copy"
	"--minimized-build"
	"--enable-modules=auto_rng,sha2_32,stream,keypair,pubkey,ecdh,x509,bcrypt"
	"--without-documentation"
)
# Compile amalgamation file
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
	list(APPEND ${project}_LIBS Crypt32 winmm ws2_32 Iphlpapi)
elseif(LINUX)
	list(APPEND ${project}_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/systemid_linux.cpp)
endif()

add_library(${project} STATIC
	${${project}_SOURCES}
	${${project}_HEADERS}
)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${INCLUDE_DIRS})

# Disable "deprecated header" messages
target_compile_definitions(${project} PRIVATE BOTAN_NO_DEPRECATED_WARNINGS)

# Link libraries
target_link_libraries(${project} ${${project}_LIBS})

set(GLOB_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include
	${CMAKE_CURRENT_BINARY_DIR}

	CACHE PATH "List of library directories for header includes"
)
