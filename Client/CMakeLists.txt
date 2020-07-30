cmake_minimum_required(VERSION 3.5)

set(project Client)
project(
	${project}
	DESCRIPTION "Gui application"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# ---Qt---
find_package(
	Qt5
	COMPONENTS
		Core
		Gui
		OpenGL
		Widgets
		SerialPort
		Concurrent
	REQUIRED
)
if (NOT Qt5Core_FOUND)
	message(FATAL_ERROR "No Qt installation found, aborting")
endif()
set(Qt5_VERSION ${Qt5Core_VERSION})
set(Qt5_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5SerialPort_LIBRARIES} ${Qt5Concurrent_LIBRARIES})
set(Qt5_DIRS ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5SerialPort_DIRS} ${Qt5Concurrent_INCLUDE_DIRS})
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
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
list(APPEND INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


# ---OpenVR---
if(LINUX)
	set(OVR_LIB linux)
elseif(WINDOWS)
	set(OVR_LIB win)
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(OVR_LIB ${OVR_LIB}64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(OVR_LIB ${OVR_LIB}32)
endif()
list(APPEND LIBRARIES openvr_api)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${OVR_LIB})


# ---Build---
set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/serial.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/procmon.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/loginwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.cpp
)
set(${project}_MOC
	${CMAKE_CURRENT_SOURCE_DIR}/client.h
	${CMAKE_CURRENT_SOURCE_DIR}/serial.h
	${CMAKE_CURRENT_SOURCE_DIR}/procmon.h
	${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/loginwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.h
	${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.h
)
set(${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

include_directories(${INCLUDE_DIRS})

add_executable(${project}
	main.cpp
	${${project}_RESOURCES}
	${${project}_SOURCES}
	${${project}_MOC}
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${project} PROPERTY WIN32_EXECUTABLE true)
endif()

set_target_properties(${project} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries(${project} ThorQLib ${LIBRARIES})

# Pure cancer
if (WINDOWS)
    if (EXISTS "C:\\Program Files (x86)\\Windows Kits\\8.1\\bin\\x64\\rc.exe")
	set(RC_EXE_PATH "C:\\Program Files (x86)\\Windows Kits\\8.1\\bin\\x64\\rc.exe")
    else()
	set(KNOWN_WIN_VERSIONS "10.0.18362.0" "10.0.17763.0" "10.0.17134.0" "10.0.16299.0" "10.0.15063.0" "10.0.14393.0" "10.0.13393.0")
	foreach(x ${KNOWN_WIN_VERSIONS})
	    if (EXISTS "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\${x}\\x64\\rc.exe")
		set(RC_EXE_PATH "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\${x}\\x64\\rc.exe")
		break()
	    endif()
	endforeach()
    endif()

    # Crossing fingers....
    execute_process(
	    COMMAND ${RC_EXE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc
	    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	    RESULT_VARIABLE py_result
	    OUTPUT_VARIABLE py_output
    )

    # Print the output
    #message("BUILD OUTPUT:\n${py_output}")

    # If it returned something else than 0 it probably didnt succeed
    if (NOT ${py_result} EQUAL 0)
	    message(FATAL_ERROR "Failed to build windows reources")
    endif()

    SET_TARGET_PROPERTIES(${project} PROPERTIES LINK_FLAGS ${CMAKE_CURRENT_SOURCE_DIR}/resources.res)
endif()
