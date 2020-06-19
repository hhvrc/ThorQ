cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(${project})
message("Configuring ${project}...")

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
	"--minimized-build" # Remove this and replace with "--disable-modules=pkcs11" if it errors
	"--without-documentation"
	"--enable-modules=hex,ecdh,chacha,pubkey,base64,bcrypt,system_rng,stream,kdf2"
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

add_library(${project} STATIC
	${CMAKE_BINARY_DIR}/botan_all.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
)

set(LIB_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include
	${CMAKE_BINARY_DIR}

	CACHE PATH "List of library directories for header includes"
)
