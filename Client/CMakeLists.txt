cmake_minimum_required(VERSION 3.5)

set(project ThorqClient)

project(${project} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
	message("Building for LINUX")
elseif(WIN32)
	set(WINDOWS TRUE)
	message("Building for Windows")
endif()

if(LINUX)
	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
		message("With full optimization (Release build)")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g0")
	else()
		message("With no optimization (Debug build)")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g3")
	endif()

	# Very verbose!
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --verbose -Wall -Wno-long-long -pedantic")

	link_directories(/usr/local/lib/)
	include_directories(/usr/include/)
	include_directories(/usr/local/include/botan-2/)

	find_library(BOTAN2 NAMES botan2 botan-2 PATHS /usr/lib/ /usr/local/lib/)
elseif(WINDOWS)
	if(${CMAKE_BUILD_TYPE} MATCHES "Release")
		message("With full optimization (Release build)")
		set(WIN_DEBUG_FLAG "/DEBUG:NONE")
	else()
		message("With no optimization (Debug build)")
		set(WIN_DEBUG_FLAG "/DEBUG:FULL")
	endif()

	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()

	# Force to always compile with MD
	if(CMAKE_CXX_FLAGS MATCHES "/MD.?")
		string(REGEX REPLACE "/MD.?" "/MD" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
	endif()
	# Force to always compile with DEBUG:FULL
	if(CMAKE_CXX_FLAGS MATCHES "/DEBUG\:[^\s]+")
		string(REGEX REPLACE "/DEBUG\:[^\s]+" "${WIN_DEBUG_FLAG}" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WIN_DEBUG_FLAG}")
	endif()

	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/windows)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/windows/include)

	find_library(BOTAN NAMES botan)
endif()

add_executable(${project}
	enet.h
	enums.h
	crypto.h
	crypto.cpp
	main.cpp
	)

if(LINUX)
	target_link_libraries(${project} botan-2)
elseif(WINDOWS)
	target_link_libraries(${project} botan)
endif()
