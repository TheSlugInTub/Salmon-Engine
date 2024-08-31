#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Window class for the engine
// Uses GLFW to make a window
class Window
{
public:
	Window(const char* title, int width, int height, bool fullscreen);
	~Window();

	// The update function swaps buffers and polls events
	void Update();
	// Returns the aspect ratio of the window, (float)width / (float)height
	float GetAspectRatio();
	// Sets the title of the window
	void SetTitle(const char* newTitle);
	// Returns true if the window closes, used for the main loop
	bool ShouldClose();
	
	GLFWwindow* window;
private:
	unsigned int width, height;
	const char* title;
};