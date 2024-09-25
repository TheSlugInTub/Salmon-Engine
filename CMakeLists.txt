# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Salmon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Specify the C++ standard (optional but recommended)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the include directory so that it is available globally
include_directories(
    include
)

# Gather all source files recursively
file(GLOB_RECURSE SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE INCLUDESOURCES include/*.cpp include/*.c)

# Comment to reload CMake, Counter: 20

# Add the executable
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${INCLUDESOURCES}
    )

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE JPH_DEBUG_RENDERER)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ../lib/glfw3
    ../lib/Jolt
    ../lib/assimp-vc143-mt
)
