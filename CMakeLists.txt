cmake_minimum_required(VERSION 3.13)

project(networked)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)



add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

add_subdirectory(3rdParty)


# add_subdirectory(w1)
# add_subdirectory(w2)
# add_subdirectory(w3)
# add_subdirectory(w4)
# add_subdirectory(w5)
# add_subdirectory(w7)
# add_subdirectory(w10)

