# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

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

# Gather source files excluding stb.cpp
file(GLOB_RECURSE SOURCES_EXCEPT_STB src/*.cpp src/*.c)
file(GLOB_RECURSE INCLUDESOURCES include/*.cpp include/*.c)
list(FILTER SOURCES_EXCEPT_STB EXCLUDE REGEX ".*stb\\.cpp$")

# Enable Unity Build
set_property(GLOBAL PROPERTY USE_UNITY_BUILD TRUE)

# Add the executable
add_executable(${PROJECT_NAME} 
    ${SOURCES_EXCEPT_STB}
    ${INCLUDESOURCES}
    "${CMAKE_SOURCE_DIR}/src/stb.cpp"
)

# Set Unity Build properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    UNITY_BUILD TRUE
    UNITY_BUILD_BATCH_SIZE 0  # Combine all files into one batch (except stb.cpp)
)

# Make sure stb.cpp is compiled separately and last
set_source_files_properties(${STB_SOURCE} PROPERTIES
    UNITY_BUILD_EXCLUDE TRUE  # Exclude from unity build
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_compile_definitions(${PROJECT_NAME} PRIVATE JPH_DEBUG_RENDERER)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/glfw3.lib
        ${CMAKE_SOURCE_DIR}/lib/Jolt.lib
        ${CMAKE_SOURCE_DIR}/lib/assimp-vc143-mt.lib
        ${CMAKE_SOURCE_DIR}/lib/OpenAL32.lib
        ${CMAKE_SOURCE_DIR}/lib/freetype.lib
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
        ${CMAKE_SOURCE_DIR}/lib/libJolt.a
        ${CMAKE_SOURCE_DIR}/lib/libassimp.so
        ${CMAKE_SOURCE_DIR}/lib/libopenal.so
        ${CMAKE_SOURCE_DIR}/lib/libfreetype.a
    )
endif()
