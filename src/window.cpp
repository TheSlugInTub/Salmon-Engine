#include <salmon/window.h>
#include <salmon/engine.h>
#include <iostream>

Window::Window(const char* title, int width, int height, bool fullscreen, bool maximize)
   : title(title), width(width), height(height)
{
    // Glfw: Initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Glfw window creation
    // --------------------
    window =
        glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    if (maximize)
        glfwMaximizeWindow(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(1);
    }

    engineState.window = this;
}

Window::~Window()
{
    glfwTerminate();
}

void Window::Update()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

float Window::GetAspectRatio()
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if (width == 0 || height == 0)
    {
        // Handle the minimized window case
        return 1.0f;
    }

    return (float)width / (float)height;
}

void Window::SetTitle(const char* newTitle)
{
    glfwSetWindowTitle(window, newTitle);
}

bool Window::ShouldClose()
{
    return glfwWindowShouldClose(window);
}
