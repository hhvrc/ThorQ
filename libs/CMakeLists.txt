cmake_minimum_required(VERSION 3.5)

set(project ThorQLib)
project(
        ${project}
	DESCRIPTION "Files/Libraries that is used by both the server and the client"
	LANGUAGES CXX
)

message("Configuring ${project}...")

add_subdirectory(fmt)

# ------ Get OpenSSL ------
find_package(OpenSSL REQUIRED)
set(${project}_LIBS
    OpenSSL::Crypto
    OpenSSL::SSL
)
if(WIN32)
    list(APPEND ${project}_LIBS Crypt32 winmm ws2_32 Iphlpapi)
endif()
# -------------------------

# ------- Get Botan -------
set(PYTHON_ARGS
    "${CMAKE_CURRENT_SOURCE_DIR}/botan/configure.py"
    "--os=${BUILD_OSL}"
    "--cc=${BUILD_COMPILER}"
    "--cpu=x86_${VPTR_BITS}"
    "--amalgamation"
    "--disable-shared"
    "--link-method=copy"
    "--minimized-build"
    "--enable-modules=auto_rng,bcrypt"
    "--without-documentation"
)
message("Executing: python ${PYTHON_ARGS}")
execute_process(
    COMMAND "python" ${PYTHON_ARGS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RESULT_VARIABLE py_result
    OUTPUT_VARIABLE py_output
)
if (NOT ${py_result} EQUAL 0)
    message("BUILD OUTPUT:\n${py_output}")
    message(FATAL_ERROR "Failed to build Botan")
endif()

# Stop botan from fucking up
string(APPEND CMAKE_CXX_FLAGS " -DBOTAN_IS_BEING_BUILT -DBOTAN_NO_DEPRECATED_WARNINGS")

# Stop botan from fucking up (on windows)
if(WIN32)
    string(APPEND CMAKE_CXX_FLAGS " -bigobj")
endif()
# -------------------------

# ------ flatbuffers ------

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

find_program (FLATBUFFERS_COMPILER flatc)
file(GLOB ${project}_SCHEMAS "${CMAKE_CURRENT_SOURCE_DIR}/schemas/*.fbs")
set(FLATC_OUTPUT_DIR  "${CMAKE_CURRENT_BINARY_DIR}/schemas")
foreach(SCHEMA ${${project}_SCHEMAS})
    get_filename_component(SCHEMA_FILE ${SCHEMA} NAME)
    get_filename_component(SCHEMA_NAME ${SCHEMA} NAME_WE)
    set(FLATC_OUTPUT_FILE "${FLATC_OUTPUT_DIR}/${SCHEMA_NAME}_generated.h")
    add_custom_command(
        OUTPUT ${FLATC_OUTPUT_FILE}
        COMMAND ${FLATBUFFERS_COMPILER}
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
    ${CMAKE_CURRENT_BINARY_DIR}/botan_all.h
    ${CMAKE_CURRENT_SOURCE_DIR}/enet/include/enet.h

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

set(GLOB_INCLUDE_DIRS
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/enet/include
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

# Get pthreads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(${project}
    Threads::Threads
    ${CMAKE_DL_LIBS}
    ${${project}_LIBS}
    flatbuffers
    fmt
)
