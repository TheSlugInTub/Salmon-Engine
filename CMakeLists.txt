# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(Salmon)

# Specify the C++ standard (optional but recommended)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the include directory so that it is available globally
include_directories(include)

# Gather all source files
file(GLOB SOURCES src/*.cpp src/*.c)

# Add the executable
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    )

# Link appropriate libraries with the operating system

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    message(STATUS "Configuring for Windows")
    target_link_libraries(${PROJECT_NAME} 
        ../lib/glfw3
    )
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "Configuring for Linux")
    target_link_libraries(${PROJECT_NAME} 
        ../lib/libglfw3
    )
endif()