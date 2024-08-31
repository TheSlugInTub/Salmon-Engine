# Salmon Engine 0.08
30/8/2024 - August

I thought about using Box2D as the physics engine, but I decided to make my own.
I am following a book called 'Physics engine design' by Ian Millington,
he makes a 3d physics engine in the book.
The renderer is going to display 3d cubes instead of quads for now.
I'll make the physics engine 2d in the future.

# Salmon Engine 0.05
30/8/2024 - August

A basic 2D OpenGL renderer has been added to the engine alongside a SpriteRenderer component,
and a transform component. There's a new camera class for rendering and two new shaders.
A vertex shader, and a fragment shader. The renderer renders all entities with four vertices,
like any 2D renderer would, but the renderer uses perspective projection instead of orthographic,
so it renders 2D entities in a 3D space, this will be helpful in the future for rendering 3D.
There is a also a new scene_manager.h file, which only has the current scene in it for now.

# Salmon Engine 0.02
29/8/2024 - August

There were some compiler errors, it's baffling how I didn't check if the code actually compiled.
Also there is a new libglfw3.a for linux, and a new window class for making a window with GLFW.

# Salmon Engine 0.01
29/8/2024 - August

Initial commit of the project. A simple ECS is implemented in the ecs.h file.
I'll have a list of registered components, from which you can add to an entity in the editor.
I obviously haven't made it that far yet.