cmake_minimum_required(VERSION 3.5)

set(project libs)

project(${project} LANGUAGES CXX)

if(LINUX)
	link_directories(/usr/local/lib/)
	include_directories(/usr/include/)
	include_directories(/usr/local/include/botan-2/)

	find_library(BOTAN2 NAMES botan2 botan-2 PATHS /usr/lib/ /usr/local/lib/)
elseif(WINDOWS)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/botan/windows)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/botan/windows/include)

	find_library(BOTAN NAMES botan)
endif()


add_library(${project} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h
	)

if(LINUX)
	target_link_libraries(${project} botan-2)
elseif(WINDOWS)
	target_link_libraries(${project} botan)
endif()
