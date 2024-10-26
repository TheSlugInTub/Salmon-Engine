#pragma once

// ImGui layer to make it easier to integrate imgui into your project

namespace ImGuiLayer
{

// Initializes ImGui (Only call at the beginning of program)
void Init();
// Creates a new ImGui frame (Only call at the beginning of your main loop)
void NewFrame();
// Ends the ImGui frame (Only call at the end of your main loop, when all the imgui components have been rendered)
void EndFrame();
// Terminates ImGui (Only call at end of program)
void Terminate();

}
