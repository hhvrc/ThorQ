cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
        ${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# ---------- fmt ----------

add_subdirectory(fmt)

# -------------------------

# --------- enet ---------

set(ENET_STATIC ON)
add_definitions(-DENET_FEATURE_ADDRESS_MAPPING)
add_subdirectory(enet)

# -------------------------

# ------ flatbuffers ------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFLATBUFFERS_USE_STD_SPAN") # C++20 :D

# Save flags
set(PREV_FLAGS ${CMAKE_CXX_FLAGS})

# Remove warning flags
string(REGEX REPLACE " ?-W.[a-zA-Z0-9=-]+| ?-pedantic" "" CMAKE_CXX_FLAGS ${PREV_FLAGS})

set(FLATBUFFERS_INSTALL      OFF CACHE BOOL "Enable the installation of targets."          FORCE)
set(FLATBUFFERS_BUILD_FLATC  OFF CACHE BOOL "Enable the build of the flatbuffers compiler" FORCE)
set(FLATBUFFERS_STATIC_FLATC ON  CACHE BOOL "Build flatbuffers compiler with -static flag" FORCE)
set(FLATBUFFERS_BUILD_TESTS  OFF CACHE BOOL "Enable the build of tests and samples."       FORCE)
add_subdirectory(flatbuffers)

# Restore flags
set(CMAKE_CXX_FLAGS ${PREV_FLAGS})

find_program (FLATC_EXEC flatc)

if(${FLATC_EXEC} STREQUAL "FLATC_EXEC-NOTFOUND")
    message(FATAL_ERROR "Could not find flatbuffers compiler, please make sure it exists in path!")
endif()

file(GLOB ${project}_SCHEMAS "${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.fbs")
set(FLATC_OUTPUT_DIR  "${CMAKE_CURRENT_BINARY_DIR}/schemas")
foreach(SCHEMA ${${project}_SCHEMAS})
    get_filename_component(SCHEMA_FILE ${SCHEMA} NAME)
    get_filename_component(SCHEMA_NAME ${SCHEMA} NAME_WE)
    set(FLATC_OUTPUT_FILE "${FLATC_OUTPUT_DIR}/${SCHEMA_NAME}_generated.h")
    add_custom_command(
        OUTPUT ${FLATC_OUTPUT_FILE}
	COMMAND ${FLATC_EXEC}
        ARGS --cpp
        ARGS --force-empty
        ARGS --gen-mutable
        ARGS --cpp-str-type "std::string"
        ARGS -o ${FLATC_OUTPUT_DIR}
        ARGS -b ${SCHEMA_FILE}
        COMMENT "Building C++ header for ${SCHEMA_FILE}"
        DEPENDS ${SCHEMA}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/schemas"
    )
    set_property(SOURCE ${FLATC_OUTPUT_FILE} APPEND PROPERTY OBJECT_DEPENDS ${SCHEMA})
    list(APPEND ${project}_SCHEMAS ${FLATC_OUTPUT_FILE})
endforeach()
# -------------------------

set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hashing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid_${BUILD_OSL}.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/crypto.h
    ${CMAKE_CURRENT_SOURCE_DIR}/hashing.h
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/systemid.h

    ${CMAKE_CURRENT_SOURCE_DIR}/enums.h
    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_global.h
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thorq_message.h
    ${${project}_SCHEMAS}
)

if (WIN32)
    set(GLOB_LINK_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/sodium/lib/windows

	CACHE PATH "List of library directories that contains binaries to link agaisnt"
    )
else()
    set(GLOB_LINK_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/sodium/lib/linux

	CACHE PATH "List of library directories that contains binaries to link agaisnt"
    )
endif()

link_directories(
    ${GLOB_LINK_DIRS}
)

set(GLOB_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/enet/include
    ${CMAKE_CURRENT_SOURCE_DIR}/sodium/include
    ${CMAKE_CURRENT_SOURCE_DIR}/flatbuffers/include

    CACHE PATH "List of library directories for header includes"
)
include_directories(
    ${GLOB_INCLUDE_DIRS}
)

add_library(${project} STATIC
    ${${project}_SOURCES}
    ${${project}_HEADERS}
)

if(WIN32)
    set(WIN_LIBS Iphlpapi)
endif()

# Get pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(${project}
    Threads::Threads
    ${CMAKE_DL_LIBS}
    ${WIN_LIBS}
    flatbuffers
    enet_static
    sodium
    fmt
)
