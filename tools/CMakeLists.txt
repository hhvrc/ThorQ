cmake_minimum_required (VERSION 3.5)

set (project Tools)
project (
    ${project}
    DESCRIPTION "A collection of tools"
    LANGUAGES CXX
)

message ("Configuring ${project}...")

add_subdirectory (keygen_encryption)
add_subdirectory (keygen_signing)
