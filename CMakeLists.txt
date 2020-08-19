cmake_minimum_required(VERSION 3.5)

set(project ThorQ)
project(
	${project}
	DESCRIPTION "Software to control 3rd party collars"
)

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
	set(OS_STR "Linux")
elseif(WIN32)
	set(WINDOWS TRUE)
	set(OS_STR "Windows")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(LINUX)
	# 64-bit, threading, and verbose
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -pthread -Wall -Wextra -pedantic")

	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector -O3 -g0")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
	endif()

	# Very verbose!
	if (VERBOSE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --verbose")
	endif()
elseif(WINDOWS)
	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
		set(WIN_DEBUG_FLAG "/DEBUG:NONE")
		set(WIN_OPTIMIZATION_FLAG "/O2")
	else()
		set(WIN_DEBUG_FLAG "/DEBUG:FULL")
		set(WIN_OPTIMIZATION_FLAG "/Od")
	endif()

	# Stop botan from fucking up
	add_definitions(/DNOMINMAX /DBOTAN_IS_BEING_BUILT)

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

	# Botan build is pretty big
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()

add_subdirectory(libs)
include_directories(${LIB_INC_DIRS})
add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(Keygen)

message("Building ${project} in ${CMAKE_BUILD_TYPE} mode for ${OS_STR}")
