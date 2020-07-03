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
        Concurrent
    REQUIRED
)
if (NOT Qt5Core_FOUND)
    message(FATAL_ERROR "No Qt installation found, aborting")
endif()
set(Qt5_VERSION ${Qt5Core_VERSION})
set(Qt5_LIBS ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5Concurrent_LIBRARIES})
set(Qt5_DIRS ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Concurrent_INCLUDE_DIRS})
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
    ${CMAKE_CURRENT_SOURCE_DIR}/procmon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/overlaywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.cpp
)
set(${project}_MOC
    ${CMAKE_CURRENT_SOURCE_DIR}/client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/procmon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/overlaywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.h
)

include_directories(${INCLUDE_DIRS})

add_executable(${project}
    main.cpp
    qml.qrc
    ${${project}_SOURCES}
    ${${project}_MOC}
)

set_target_properties(${project} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries(${project} ThorQLib ${LIBRARIES})
