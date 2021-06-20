cmake_minimum_required (VERSION 3.5)

set (project tools-keygen-signing)
project (
    ${project}
    DESCRIPTION "Generates a keypair for signing messages"
    LANGUAGES CXX
)

message ("Configuring ${project}...")

add_executable (${project} main.cpp)

set_target_properties (${project} PROPERTIES COMPILE_FLAGS -DBUILDER_STATIC_DEFINE)
target_link_libraries (${project} ThorQLib)
