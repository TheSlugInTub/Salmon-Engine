# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Salmon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specify the C++ standard (optional but recommended)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# Use the static runtime libraries
set(CMAKE_CXX_FLAGS_DEBUG "/MT")
set(CMAKE_CXX_FLAGS_RELEASE "/MT")

# Add the include directory so that it is available globally
include_directories(
    include
)

# Gather all source files recursively
file(GLOB_RECURSE SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE INCLUDESOURCES include/*.cpp include/*.c)

# Comment to reload CMake, Counter: 17

# Add the executable
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${INCLUDESOURCES}
    )

# Link appropriate libraries with the operating system

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "Configuring for Windows")
    target_link_libraries(${PROJECT_NAME} 
        ../lib/glfw3
        ../lib/assimp-vc143-mtd
        ../lib/Jolt
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Configuring for Linux")
    target_link_libraries(${PROJECT_NAME} 
        ../lib/libglfw3.a
    )
endif()
