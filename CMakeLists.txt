cmake_minimum_required(VERSION 3.5)

set(project ThorQ)
project(
	${project}
	DESCRIPTION "Software to control 3rd party collars"
)

option(VERBOSE "Verbose compiler output" OFF)

# --- Configure default build ---
set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
# --------------------------------

# --- Simplify build checking ---
math(EXPR VPTR_BITS "${CMAKE_SIZEOF_VOID_P}*8") # Convert 4/8 -> 32/64

if(UNIX AND NOT APPLE)
	set(BUILD_OS "linux")
	set(BUILD_OSL "linux")
elseif(WIN32)
	set(BUILD_OS "win")
	set(BUILD_OSL "windows")
endif()

string(TOLOWER ${CMAKE_CXX_COMPILER_ID} BUILD_COMPILER)

# Translate some stuff
if (${BUILD_COMPILER} STREQUAL "gnu")
  set(BUILD_COMPILER "gcc")
elseif (${BUILD_COMPILER} STREQUAL "intel")
  set(BUILD_COMPILER "icc")
endif()
# --------------------------------

# --- Find and configure Qt ---
find_package(
    Qt5
    COMPONENTS
        Core
		Gui
		OpenGL
		Widgets
		SerialPort
		Concurrent
		Sql
    REQUIRED
)
if (NOT Qt5Core_FOUND)
    message(FATAL_ERROR "No Qt installation found, aborting")
endif()
message("Found Qt ${Qt5Core_VERSION}")
# -----------------------------

# --- Set global C++ CMake settings ---
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(VERBOSE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
# -------------------------------

# --- Set compiler flags ---
add_definitions(-DBUILDER_STATIC_DEFINE)
if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
    add_definitions(-DDEBUG)
endif()
if(MSVC)
    # Remove all previously declared flags before adding new ones
    string(REGEX REPLACE "[/-]W[0-4] ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-][ML][DT]d? ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-]O[12bdgistxy] ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-]DEBUG:[a-zA-Z]+ ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    # Enable max warnings
    string(APPEND CMAKE_CXX_FLAGS " -W4")

    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	string(APPEND CMAKE_CXX_FLAGS " -MT -DEBUG:NONE -O2")
    else()
	string(APPEND CMAKE_CXX_FLAGS " -MTd -DEBUG:FULL -Od")
    endif()
else()
    # 64-bit, threading, and verbose
    string(APPEND CMAKE_CXX_FLAGS " -m64 -pthread -Wall -Wextra -pedantic")

    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	string(APPEND CMAKE_CXX_FLAGS " -fstack-protector -O3 -g0 -s")
    else()
	string(APPEND CMAKE_CXX_FLAGS " -O0 -g3")
    endif()
endif()
# --------------------------

add_subdirectory(libs)
include_directories(${GLOB_INCLUDE_DIRS})
add_subdirectory(Client)
add_subdirectory(Server)
add_subdirectory(Keygen)

message("Building ${project} in ${CMAKE_BUILD_TYPE} mode for ${BUILD_OS} x86_${VPTR_BITS} with ${BUILD_COMPILER}")
