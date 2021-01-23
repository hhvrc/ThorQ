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

# --- Set global C++ CMake settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
if(VERBOSE)
    set(CMAKE_VERBOSE_MAKEFILE ON)
endif()
# -------------------------------

# --- Set compiler flags ---
# Debugging, and warnings
add_definitions(-DBUILDER_STATIC_DEFINE)
if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Release")
    add_definitions(-DDEBUG)

    if(MSVC)
        # Remove all previously declared flags before adding new ones
        string(REGEX REPLACE "[/-]W[0-4] ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

        # Verboseeeee
        string(APPEND CMAKE_CXX_FLAGS " -Wall")
    else()
        # Verboseeeee
        string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra -pedantic -Wformat=2 -Wsign-conversion -Wundef -Wredundant-decls -Wwrite-strings -Wpointer-arith -Wcast-qual -Wmissing-include-dirs -Wcast-align -Wctor-dtor-privacy -Wdisabled-optimization -Winvalid-pch -Woverloaded-virtual -Wconversion -Wswitch-enum -Wno-ctor-dtor-privacy -Wno-format-nonliteral -Wnoexcept -Wno-dangling-else -Wno-unused-local-typedefs -Wdouble-promotion -Wtrampolines -Wzero-as-null-pointer-constant -Wuseless-cast -Wvector-operation-performance -Wsized-deallocation -Wshadow -Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond")
    endif()
endif()

# Optimization and compiler specific fixes
if(MSVC)
    # Remove all previously declared flags before adding new ones
    string(REGEX REPLACE "[/-][ML][DT]d? ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-]O[12bdgistxy] ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE "[/-]DEBUG:[a-zA-Z]+ ?" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

    # Enable max warnings (But dont spam the console)
    string(APPEND CMAKE_CXX_FLAGS " -DNOMINMAX -wo9025 -wo5054")

    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	string(APPEND CMAKE_CXX_FLAGS " -MT -DEBUG:NONE -O2")
    else()
	string(APPEND CMAKE_CXX_FLAGS " -MTd -DEBUG:FULL -Od")
    endif()
else()
    # 64-bit, and threading
    string(APPEND CMAKE_CXX_FLAGS " -m64 -pthread")

    if(${CMAKE_BUILD_TYPE} MATCHES "Release")
	string(APPEND CMAKE_CXX_FLAGS " -fstack-protector -O3 -g0 -s")
    else()
	string(APPEND CMAKE_CXX_FLAGS " -O0 -g3")
    endif()
endif()
# --------------------------

# -- Set global libraries --
if(WIN32)
    link_libraries(ws2_32 winmm Rpcrt4)
endif()
# --------------------------

add_subdirectory(libs)
link_directories(${GLOB_LINK_DIRS})
include_directories(${GLOB_INCLUDE_DIRS})
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(cryptotest)

message("Building ${project} in ${CMAKE_BUILD_TYPE} mode for ${VPTR_BITS}-bit ${BUILD_OSL}, using ${BUILD_COMPILER}")
