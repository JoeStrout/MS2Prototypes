cmake_minimum_required(VERSION 3.15)
project(GCApproach2Cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(GCApproach2Cpp
    Demo.cpp
    GCItems.cpp
    GCManager.cpp
)

target_compile_options(GCApproach2Cpp PRIVATE -Wall -Wextra -O2)
