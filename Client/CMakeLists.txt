cmake_minimum_required(VERSION 3.5)

set(project Client)
project(
	${project}
	DESCRIPTION "Gui application"
	LANGUAGES CXX
)

message("Configuring ${project}...")

# --- OpenVR ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/openvr/headers)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/openvr/lib/${BUILD_OS}${VPTR_BITS}) # eg. win64 / linux32
# --------------

# --- Build ---
set(${project}_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/user.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/client.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/serial.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/procmon.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/loginwidget.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/userlistitem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/userlistwidget.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.cpp
)
set(${project}_HEADERS
	${CMAKE_CURRENT_SOURCE_DIR}/user.h
	${CMAKE_CURRENT_SOURCE_DIR}/client.h
	${CMAKE_CURRENT_SOURCE_DIR}/serial.h
	${CMAKE_CURRENT_SOURCE_DIR}/procmon.h
	${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/loginwidget.h

	${CMAKE_CURRENT_SOURCE_DIR}/userlistitem.h
	${CMAKE_CURRENT_SOURCE_DIR}/userlistwidget.h

	${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.h
	${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.h
	${CMAKE_CURRENT_SOURCE_DIR}/openvroverlaycontroller.h
)
set(${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

if (WIN32)
    list(APPEND ${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif()

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5SerialPort_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${project}
        main.cpp
	${${project}_HEADERS}
	${${project}_SOURCES}
	${${project}_RESOURCES}
)

target_link_libraries(${project}
    ThorQLib
    openvr_api
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5OpenGL_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5SerialPort_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${project} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
