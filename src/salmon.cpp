#include <iostream>
#include <ecs.h>
#include <components.h>
#include <window.h>
#include <utils.h>
#include <scene_manager.h>
#include <renderer.h>
#include <chrono>

Scene scene;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

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

    Window window("Slug's Window", 1920, 1080, false);
    Utils::globalWindow = &window;
    Utils::globalCamera = &camera;

    EntityID ent = scene.AddEntity();
    scene.Assign<SpriteRenderer>(ent);
    scene.Assign<Transform>(ent);
    scene.Get<SpriteRenderer>(ent)->texture = Utils::LoadTexture("res/Slugarius.png");

    Renderer::Init();

    while (!window.ShouldClose())
    {
#pragma region FPS
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        lastTime = currentTime;

        // Update frame count
        frameCount++;

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

        UpdateSystems();

        window.Update();
    }

    return 0;
}
