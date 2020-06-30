cmake_minimum_required(VERSION 3.5)

set(project Client)
project(
	${project}
	DESCRIPTION "Gui application"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message("Configuring ${project}...")

# ---Qt---
find_package(
	Qt5
	COMPONENTS
	    Core
		Gui
		Widgets
		Concurrent
	REQUIRED
)
if (NOT Qt5Core_FOUND)
	message(FATAL_ERROR "No Qt installation found, aborting")
endif()
set(Qt5_VERSION ${Qt5Core_VERSION})
set(Qt5_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Concurrent_LIBRARIES})
set(Qt5_DIRS ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Concurrent_INCLUDE_DIRS})
message("Found Qt ${Qt5_VERSION}")
message("Includes:\t\t${Qt5_DIRS}")
message("Libraries:\t\t${Qt5_LIBS}\n")
list(APPEND LIBRARIES ${Qt5_LIBS})
list(APPEND INCLUDE_DIRS ${Qt5_DIRS})
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
set(CMAKE_AUTOMOC ON)
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/client.cpp
)
set(${project}_MOC
	${CMAKE_CURRENT_SOURCE_DIR}/client.h
)

include_directories(${INCLUDE_DIRS})
link_directories(${LIBRARY_DIRS})
add_executable(${project} main.cpp ${${project}_SOURCES} ${${project}_MOC})
set_target_properties(${project} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries(${project} ThorQLib ${LIBRARIES})
