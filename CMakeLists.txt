cmake_minimum_required(VERSION 3.5)

project(ThorQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
enable_language(CXX)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
	message("Building for LINUX")
elseif(WIN32)
	set(WINDOWS TRUE)
	message("Building for Windows")
endif()

if(LINUX)
	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
		message("With full optimization (Release build)")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g0")
	else()
		message("With no optimization (Debug build)")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
	endif()

	# Very verbose!
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --verbose -Wall -Wno-long-long -pedantic")
elseif(WINDOWS)
	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
		message("With full optimization (Release build)")
		set(WIN_DEBUG_FLAG "/DEBUG:NONE")
		set(WIN_OPTIMIZATION_FLAG "/O2")
	else()
		message("With no optimization (Debug build)")
		set(WIN_DEBUG_FLAG "/DEBUG:FULL")
		set(WIN_OPTIMIZATION_FLAG "/Od")
	endif()

	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()

	# Force to always compile with MT
	if(CMAKE_CXX_FLAGS MATCHES "/MDd?|/MTd?|/LDd?")
		string(REGEX REPLACE "/MDd?|/MTd?|/LDd?" "/MT" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
	endif()

	# Force to always compile with /O2 or /Od
	if(CMAKE_CXX_FLAGS MATCHES "/O.?")
		string(REGEX REPLACE "/O.?" ${WIN_OPTIMIZATION_FLAG} CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WIN_OPTIMIZATION_FLAG}")
	endif()

	# Force to always compile with DEBUG:XXXX
	if(CMAKE_CXX_FLAGS MATCHES "/DEBUG\:[^\\s]+")
		string(REGEX REPLACE "/DEBUG\:[^\\s]+" ${WIN_DEBUG_FLAG} CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WIN_DEBUG_FLAG}")
	endif()
endif()

add_subdirectory(libs)
include_directories(${LIB_INC_DIRS})
add_subdirectory(Client)
add_subdirectory(Server)
