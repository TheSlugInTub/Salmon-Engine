#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	Window(const char* title, int width, int height, bool fullscreen);
	~Window();

	void Update();
	float GetAspectRatio();
	void SetTitle(const char* newTitle);
	bool ShouldClose();
	
	GLFWwindow* window;
private:
	unsigned int width, height;
	const char* title;
};