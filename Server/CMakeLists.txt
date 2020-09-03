cmake_minimum_required(VERSION 3.5)

set(project Server)
project(
	${project}
	DESCRIPTION "Server to connect users"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# ---Qt---
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
include_directories(${Qt5_DIRS})


set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/instance.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/instancemap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/authhandler.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/eventhandlers.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.cpp
)
set(${project}_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/instance.h
	${CMAKE_CURRENT_SOURCE_DIR}/typedefs.h
	${CMAKE_CURRENT_SOURCE_DIR}/singletons.h
	${CMAKE_CURRENT_SOURCE_DIR}/instancemap.h
	${CMAKE_CURRENT_SOURCE_DIR}/authhandler.h
	${CMAKE_CURRENT_SOURCE_DIR}/eventhandlers.h
	${CMAKE_CURRENT_SOURCE_DIR}/messagehandlers.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${project}
	main.cpp
	${${project}_SOURCES}
	${${project}_HEADERS}
)

set_target_properties(${project} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries(${project} ThorQLib)
