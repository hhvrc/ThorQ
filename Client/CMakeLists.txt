cmake_minimum_required(VERSION 3.5)

set(project Client)
project(
	${project}
	DESCRIPTION "Gui application"
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

	${CMAKE_CURRENT_SOURCE_DIR}/usermodel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/userdelegate.cpp

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

	${CMAKE_CURRENT_SOURCE_DIR}/userdata.h
	${CMAKE_CURRENT_SOURCE_DIR}/usermodel.h
	${CMAKE_CURRENT_SOURCE_DIR}/userdelegate.h

	${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.h
	${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.h
)
set(${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

if (WINDOWS)
    list(APPEND ${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

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
target_link_libraries(${project} ThorQLib)
