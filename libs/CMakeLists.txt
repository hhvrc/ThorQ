cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(${project})
message("Configuring ${project}...")

set(BOTAN_H ${BUILD_PATH}/botan_all.h)
set(BOTAN_CPP ${BUILD_PATH}/botan_all.cpp)

# If the compiled source doesnt exist, then generate it
if(NOT EXISTS ${BOTAN_CPP} OR NOT EXISTS ${BOTAN_H})
	#Get relative path from build to botan source
	file(RELATIVE_PATH REL_PATH ${BUILD_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

	set(PYTHON_ARGS
		"${REL_PATH}/botan/configure.py"
		"--minimized-build"
		"--without-documentation"
		"--amalgamation"
		"--enable-modules=hex,ecdh,chacha,pubkey,base64,bcrypt,system_rng,stream"
	)

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

	# For some reason CMake fuck up on windows, so here is a stupid fix:
	if(WINDOWS)
		string (REPLACE ";" " " PYTHON_ARGS "${PYTHON_ARGS}")
	endif()
	
	file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${BUILD_PATH})
	
	message("cd ${REL_PATH}")
	message("python ${PYTHON_ARGS}")

	execute_process(
		COMMAND "python" ${PYTHON_ARGS}
		WORKING_DIRECTORY ${REL_PATH}
		RESULT_VARIABLE py_result
		OUTPUT_VARIABLE py_output
	)

    message("BUILD OUTPUT:\n${py_output}")

	if (NOT ${py_result} EQUAL 0)
		message(FATAL_ERROR "Failed to build Botan")
	endif()
endif()

set(HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/enet/include/enet.h
	${BOTAN_H}
)
add_library(${project} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
	${BOTAN_CPP}
	${HEADERS}
)

set(LIBRARY_HEADERS ${HEADERS} CACHE PATH "List of library headers (*.h, *.hpp)")
