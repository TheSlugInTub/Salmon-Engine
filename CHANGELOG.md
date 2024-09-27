# Salmon Engine 0.4
26/9/2024 - September

Skeletal animation has been integrated into the engine. There is an animation class,
and an animator component which takes in an animation class as a parameter.

# Salmon Engine 0.38
14/9/2024 - September

Shadow mapping has been fully integrated into the engine with support for multiple lights and shadows. 

# Salmon Engine 0.34
14/9/2024 - September

Shadow mapping has been added to the engine, although it's integrated quite terribly.
I'll make a better and more elegant integration later that actually handles multiple lights.
Also capsule colliders have been added in the previous commits.

# Salmon Engine 0.3
9/9/2024 - September

Jolt physics has been integrated with a RigidBody3D component, along with some functions related
to intializing and stepping jolt physics.

# Salmon Engine 0.25
6/9/2024 - September

Jolt physics has been integrated, well not really integrated it's just all in the main file.
It's going to be integrated and have its own RigidBody3D component in the next update.

# Salmon Engine 0.23
5/9/2024 - September

3d rendering in the engine with model importing with assimp. No lighting yet tho ;)

# Salmon Engine 0.2
2/9/2024 - September

Starting from scratch for performance.

# Salmon Engine 0.1
2/9/2024 - September

I have added an inspector window, it only shows the info and transform components for now.
There is also easy input in the engine now, with GetKey and GetKeyDown functions and the same
functions for mouse input as well. There is a new ScreenToWorld function in the camera class
which makes it easy to convert from screen coordinates to world coordinates

# Salmon Engine 0.099
1/9/2024 - September

I have added ImGui to the project for the editor, there are two new files an EditorLayer.h,
and a EditorLayer.cpp, this is layer between ImGui and the rest of the project.
There is an ImGui window which displays the entities in a scene.

# Salmon Engine 0.09
1/9/2024 - September

Scrap the custom physics engine, I'm just gonna use box2d, maybe another day.
There is a new shader, a line shader to display the vertices of a box2d body to make
it easier to see the colliders. There is a new Rigidbody2D component.

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
