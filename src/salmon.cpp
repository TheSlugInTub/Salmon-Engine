#include <iostream>
#include <ecs.h>
#include <components.h>
#include <window.h>
#include <utils.h>
#include <scene_manager.h>
#include <renderer.h>
#include <chrono>
#include <smphys/smphys.h>

const int screenWidth = 1920;
const int screenHeight = 1080;

void processInput(GLFWwindow* window);

// Mouse stuff.
float lastX = (float)screenWidth / 2.0;
float lastY = (float)screenHeight / 2.0;
bool firstMouse = true;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}

Scene scene;

// FPS stuff.
std::string FPS;
auto lastTime = std::chrono::high_resolution_clock::now();
int frameCount = 0;
float fps = 0.0f;
double prevTime = 0.0;
double currentTime = 0.0;
unsigned int counter = 0;

int main(int argc, char* argv[])
{
    SceneManager::SetCurrentScene(scene);

    Window window("Slug's Window", screenWidth, screenHeight, false);
    glfwSetCursorPosCallback(window.window, mouse_callback);
    Utils::globalWindow = &window;
    Utils::globalCamera = &camera;

    EntityID ent = scene.AddEntity();
    scene.Assign<SpriteRenderer>(ent);
    scene.Assign<Transform>(ent);
    scene.Assign<SmBody>(ent);
    scene.Get<SpriteRenderer>(ent)->texture = Utils::LoadTexture("res/Slugarius.png");

    EntityID ent2 = scene.AddEntity();
    scene.Assign<SpriteRenderer>(ent2);
    scene.Assign<Transform>(ent2);
    scene.Get<Transform>(ent2)->position = glm::vec3(0.0f, -3.0f, 0.0f);
    scene.Get<Transform>(ent2)->scale = glm::vec3(10.0f, 1.0f, 10.0f);
    scene.Get<SpriteRenderer>(ent2)->texture = Utils::LoadTexture("res/background.png");

    Renderer::Init();

    while (!window.ShouldClose())
    {
#pragma region FPS
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;

        // Update frame count
        frameCount++;

        // Calculate the duration between the last frame and the current frame
        std::chrono::duration<float> deltaTimeCh = currentTime - lastTime;

        // Convert duration to seconds
        float deltaTime = deltaTimeCh.count();

        // Update the last frame time
        lastTime = currentTime;

        Utils::deltaTime = deltaTime;

        // Accumulate time
        static float timeAccumulator = 0.0f;
        timeAccumulator += elapsed.count();

        // Update FPS every second
        if (timeAccumulator >= 0.1f) {
            fps = frameCount / timeAccumulator;

            // Reset counters
            frameCount = 0;
            timeAccumulator = 0.0f;

            // Update the FPS string
            FPS = std::to_string(fps);
        }
#pragma endregion
        std::cout << "FPS: " << FPS << std::endl;

        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(window.window, GLFW_KEY_T) == GLFW_PRESS)
        {
            BodyAddForce(scene.Get<SmBody>(ent), glm::vec3(10.5f, 10.5f, 0.0f));
        }

        BodyAddForce(scene.Get<SmBody>(ent), glm::vec3(0.0f, -30.0f, 0.0f));
        
        UpdateSystems();
        processInput(window.window);

        window.Update();
    }

    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::UP, 0.02f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::DOWN, 0.02f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::LEFT, 0.02f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::RIGHT, 0.02f);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::FORWARD, 0.02f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(CameraMovement::BACKWARD, 0.02f);
}