cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(${project})
message("Configuring ${project}...")

## If the compiled source doesnt exist, then generate it
#if(NOT EXISTS ${CMAKE_BINARY_DIR}/botan_all.cpp OR NOT EXISTS ${CMAKE_BINARY_DIR}/botan_all.h)
# Set python initial arguments
set(PYTHON_ARGS
	"${CMAKE_CURRENT_SOURCE_DIR}/botan/configure.py"
	"--minimized-build"
	"--without-documentation"
	"--amalgamation"
	"--enable-modules=hex,ecdh,chacha,pubkey,base64,bcrypt,system_rng,stream,kdf2"
)

# Set OS specific arguments
if(LINUX)
	set(PYTHON_ARGS
		${PYTHON_ARGS}
		"--os=linux"
		"--cc=gcc"
		"--cpu=x86_64"
	)
elseif(WINDOWS)
	set(PYTHON_ARGS
		${PYTHON_ARGS}
		"--os=windows"
		"--cc=msvc"
		"--cpu=x86_64"
		"--msvc-runtime=MT"
	)
endif()

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
#endif()

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
