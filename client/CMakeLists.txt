cmake_minimum_required (VERSION 3.5)

set (project ThorQ)
project (
    ${project}
    DESCRIPTION "Gui application"
    LANGUAGES CXX
)

message ("Configuring ${project}...")

# --- OpenVR ---
if (APPLE)
    set (OPENVR_BUILD_NAME "${BUILD_OS}32") # OpenVR only has 32-bit for OSX
else ()
    set (OPENVR_BUILD_NAME "${BUILD_OS}${VPTR_BITS}")
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/vr/openvr/headers)
link_directories (${CMAKE_CURRENT_SOURCE_DIR}/vr/openvr/lib/${OPENVR_BUILD_NAME}) # eg. win64 / linux32
file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/vr/openvr/bin/${OPENVR_BUILD_NAME}/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# --------------

# --- Find and configure Qt ---
find_package (
    Qt5
    COMPONENTS
        Core
        Gui
        OpenGL
        Widgets
        Multimedia
        MultimediaWidgets
        SerialPort
        Concurrent
        Network
    REQUIRED
)
if (NOT Qt5Core_FOUND)
    message (FATAL_ERROR "No Qt installation found, aborting")
endif ()

set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTORCC ON)

message ("Found Qt ${Qt5Core_VERSION}")
# -----------------------------

# --- Build ---
set (${project}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/user.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/procmon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/accountloginwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/accountregisterwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/accountrecoverwidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/accountcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/announcementcontroller.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/stylesheets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/errorlabel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/namedlineedit.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/collar/serial.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/apiclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/apiclient_connection.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovraction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovroverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovractionset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrinputvalue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrtrackeddevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/openvroverlaycontroller.cpp
)
set (${project}_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/user.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/procmon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/userlistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/typedefs_client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/controlwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/settingswidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/deleteexecutable.h

    ${CMAKE_CURRENT_SOURCE_DIR}/accountloginwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accountregisterwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/accountrecoverwidget.h

    ${CMAKE_CURRENT_SOURCE_DIR}/accountcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/announcementcontroller.h

    ${CMAKE_CURRENT_SOURCE_DIR}/stylesheets.h
    ${CMAKE_CURRENT_SOURCE_DIR}/errorlabel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/namedlineedit.h

    ${CMAKE_CURRENT_SOURCE_DIR}/collar/serial.h

    ${CMAKE_CURRENT_SOURCE_DIR}/apiclient.h
    ${CMAKE_CURRENT_SOURCE_DIR}/apiclient_connection.h

    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrhelpers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovraction.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovroverlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovractionset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrinputvalue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/ovrtrackeddevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vr/openvroverlaycontroller.h

    ${CMAKE_CURRENT_SOURCE_DIR}/messagecontext.h

    ${CMAKE_CURRENT_SOURCE_DIR}/declerations_qt.h
)
set (${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc)

if (WIN32)
    list (APPEND ${project}_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources.rc)
endif ()

include_directories (
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Multimedia_INCLUDE_DIRS}
    ${Qt5MultimediaWidgets_INCLUDE_DIRS}
    ${Qt5SerialPort_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable (${project}
    main.cpp
    ${${project}_HEADERS}
    ${${project}_SOURCES}
    ${${project}_RESOURCES}
)

target_link_libraries (${project} PRIVATE
    ThorQLib
    openvr_api
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5OpenGL_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Multimedia_LIBRARIES}
    ${Qt5MultimediaWidgets_LIBRARIES}
    ${Qt5SerialPort_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5Network_LIBRARIES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property (TARGET ${project} PROPERTY WIN32_EXECUTABLE TRUE)
endif ()
