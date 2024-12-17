# Salmon Engine

Salmon engine is a simple, light-weight game engine with an ECS (Entity Component System) design.

The engine includes a 3D OpenGL renderer and 3D physics with Jolt.
If you want to see an example game made with the engine, switch to the the 'syslocked' branch.

# Getting started

To compile the project:

```bash
git clone https://github.com/TheSlugInTub/Salmon-Engine.git
cd Salmon-Engine
mkdir build
cmake -S . -B build
cmake --build build --config Release 
```

Copy the 'res', 'shaders' folders and assimp DLL if you're on windows to the executable's directory.

# Libraries Used

I have used these libraries in the engine:

[Glad](https://glad.dav1d.de/) - For OpenGL and rendering. \
[GLFW](https://github.com/glfw/glfw) - For window, input and context creation. \
[stb_image](https://github.com/nothings/stb) - For image loading. \
[Jolt Physics](https://github.com/jrouwe/JoltPhysics/) - For physics. \
[openal-soft](https://github.com/kcat/openal-soft) - For audio. \
[GLM](https://github.com/g-truc/glm) - For math. \
[assimp](https://github.com/assimp/assimp) - For model loading. \
[imgui](https://github.com/ocornut/imgui) - Not neccessary in the engine but can be used for GUI. 
