cmake_minimum_required(VERSION 3.5)

set(project Library)
project(${project})
message("Configuring ${project}...")

add_library(${project} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/enet/include)

if(LINUX)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/botanc/linux)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/botanc/linux/include)
	set(BOTAN_FLAGS "--os=linux --cc=gcc")
	set(OS_BUILD_CMD "make -j4")
elseif(WINDOWS)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/botanc/windows)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/botanc/windows/include)
	set(BOTAN_FLAGS "--os=windows --cc=msvc --msvc-runtime=MT")
	set(OS_BUILD_CMD "nmake")
endif()

include_directories(${THORQ_INCLUDES})
target_link_libraries(${project} PUBLIC botan)

add_custom_command(
	OUTPUT
	    ${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/botan/src/
	COMMAND BotanConfigure "python configure.py --minimized-build --without-documentation --enable-modules=hex,ecdh,chacha,pubkey,base64,bcrypt,system_rng,stream --disable-shared-library ${BOTAN_FLAGS}"
	COMMAND BotanBuild ${OS_BUILD_CMD}
)

message("HOLY FUCK I WISH CMAKE WASNT AN ABSOLUTE FUCKING PIECE OF SHIT SCRIPTING LANGUAGE!")
