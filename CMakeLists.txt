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
file(GLOB_RECURSE SOURCES src/*.cpp src/*.c)
file(GLOB_RECURSE INCLUDESOURCES include/*.cpp include/*.c)

# Add the executable
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${INCLUDESOURCES}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable precompiled headers
set(PCH_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/salmon/stl_pch.h
    ${CMAKE_SOURCE_DIR}/include/salmon/glm_pch.h
    ${CMAKE_SOURCE_DIR}/include/salmon/assimp_pch.h
    ${CMAKE_SOURCE_DIR}/include/ft2build.h
    ${CMAKE_SOURCE_DIR}/include/salmon/physics.h
    ${CMAKE_SOURCE_DIR}/include/salmon/ecs.h
    ${CMAKE_SOURCE_DIR}/include/salmon/components.h
    ${CMAKE_SOURCE_DIR}/include/salmon/engine.h
    ${CMAKE_SOURCE_DIR}/include/salmon/bone.h
    ${CMAKE_SOURCE_DIR}/include/salmon/audiofile.h
    ${CMAKE_SOURCE_DIR}/include/salmon/mesh.h
    ${CMAKE_SOURCE_DIR}/include/salmon/model.h
    ${CMAKE_SOURCE_DIR}/include/salmon/shader.h
    ${CMAKE_SOURCE_DIR}/include/salmon/stb_image.h
    ${CMAKE_SOURCE_DIR}/include/salmon/ui.h
    ${CMAKE_SOURCE_DIR}/include/salmon/sound_buffer.h
    ${CMAKE_SOURCE_DIR}/include/salmon/sound_device.h
    ${CMAKE_SOURCE_DIR}/include/salmon/sound_source.h
    ${CMAKE_SOURCE_DIR}/include/salmon/editor.h
    ${CMAKE_SOURCE_DIR}/include/salmon/camera.h
    ${CMAKE_SOURCE_DIR}/include/salmon/animation.h
    ${CMAKE_SOURCE_DIR}/include/salmon/utils.h
    ${CMAKE_SOURCE_DIR}/include/salmon/window.h
    ${CMAKE_SOURCE_DIR}/include/glad/glad.h
    ${CMAKE_SOURCE_DIR}/include/GLFW/glfw3.h
    ${CMAKE_SOURCE_DIR}/include/salmon/imgui_pch.h
)


file(GLOB_RECURSE AUTO_PCH_HEADERS include/*.h include/*.hpp)
# Remove salmon directory headers from precompilation
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/salmon/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/Jolt/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/assimp/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/freetype/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/glm/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/imgui/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/GLFW/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/glad/.*")
list(FILTER AUTO_PCH_HEADERS EXCLUDE REGEX "include/bombratter/.*")

# Combine manually specified and auto-discovered headers
set(ALL_PCH_HEADERS ${PCH_HEADER_FILES} ${AUTO_PCH_HEADERS})

# Specify that these are C++ headers
target_precompile_headers(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:${ALL_PCH_HEADERS}>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    JPH_DEBUG_RENDERER
    )

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/glfw3.lib
        ${CMAKE_SOURCE_DIR}/lib/Jolt.lib
        ${CMAKE_SOURCE_DIR}/lib/assimp-vc143-mt.lib
        ${CMAKE_SOURCE_DIR}/lib/OpenAL32.lib
        ${CMAKE_SOURCE_DIR}/lib/freetype.lib
        ${CMAKE_SOURCE_DIR}/lib/box2d.lib
    )
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
        ${CMAKE_SOURCE_DIR}/lib/libJolt.a
        ${CMAKE_SOURCE_DIR}/lib/libassimp.so
        ${CMAKE_SOURCE_DIR}/lib/libopenal.so
        ${CMAKE_SOURCE_DIR}/lib/libfreetype.a
        ${CMAKE_SOURCE_DIR}/lib/libbox2d.a
    )
endif()
