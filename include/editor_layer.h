#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

// Editor layer, uses ImGui to draw the editor

namespace EditorLayer
{

// Selected entity index in the list of entities in the current scene
inline int selectedObjectIndex = -1;
// Font for the ImGui editor
inline ImFont* mainfont;

// Initializes ImGui for rendering
void Init();
// Makes a new ImGui frame and begins dockspace
void NewFrame();
// Renders ImGui at the end of the main loop
void Render();
// Terminates ImGui (used at end of program)
void Terminate();
// Draws the scene hierarchy
void DrawHierarchy();
// Draws the inspector, which lets you see the components of an entity
void DrawInspector();
// ImGui style theme courtesy of https://github.com/janekb04
void EmbraceTheDarkness();

}
