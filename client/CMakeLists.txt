cmake_minimum_required(VERSION 3.5)

set(project Client)
project(
    ${project}
    DESCRIPTION "Gui application"
    LANGUAGES CXX
)

message("Configuring ${project}...")

# --- OpenVR ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vr/openvr/headers)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/vr/openvr/lib/${BUILD_OS}${VPTR_BITS}) # eg. win64 / linux32
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/vr/openvr/bin/${BUILD_OS}${VPTR_BITS}/  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# --------------

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
        Network
    REQUIRED
)
if (NOT Qt5Core_FOUND)
    message(FATAL_ERROR "No Qt installation found, aborting")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

message("Found Qt ${Qt5Core_VERSION}")
# -----------------------------

# --- Build ---
set(${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/user.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serverhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/procmon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/loginwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/collar/serial.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/networking/host.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networking/message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/networking/connectionhandler.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrdevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovractionset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/openvroverlaycontroller.cpp
)
set(${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/user.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serverhandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/procmon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/loginwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.h

    ${CMAKE_CURRENT_SOURCE_DIR}/collar/serial.h

    ${CMAKE_CURRENT_SOURCE_DIR}/networking/host.h
    ${CMAKE_CURRENT_SOURCE_DIR}/networking/message.h
    ${CMAKE_CURRENT_SOURCE_DIR}/networking/connectionhandler.h

    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrdevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovraction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovractionset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/openvroverlaycontroller.h
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
    ${Qt5Network_INCLUDE_DIRS}
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
    ${Qt5Network_LIBRARIES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${project} PROPERTY WIN32_EXECUTABLE TRUE)
endif()
