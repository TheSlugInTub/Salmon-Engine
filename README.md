# Salmon Engine

Salmon engine is a simple, light-weight game engine with an ECS (Entity Component System) design.

The engine includes a 2D OpenGL renderer and 3D physics with Jolt.

# Getting started

Clone the repository using `git clone https://github.com/TheSlugInTub/Salmon-Engine.git`\
Make a build folder, cd into the build folder, type `cmake ..` to generate build files.\
Change directory to the main directory, and type `cmake --build bld` to make an executable.\
Copy the 'shaders' and 'res' folders into the directory where the executable is stored
to get it to run properly.

### DISCLAIMER:

There is an assimp library in the 'lib' folder, which is a .lib file, this will not work on linux!
I was too lazy to compile the library for linux so just compile it yourself.
