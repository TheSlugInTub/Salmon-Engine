#include <iostream>
#include <salmon/salmon.h>
#include <string>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

float physTimer = 0.3f;

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false);
    glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    StartPhysics();

    Model        boxModel("res/models/Box.obj");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");
    unsigned int slugariusTex = Utils::LoadTexture("res/textures/Slugarius.png");

    Scene scene;

    EntityID box = scene.AddEntity();
    scene.AssignParam<Transform>(box, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(box, boxModel, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), slugariusTex);
    scene.AssignParam<RigidBody3D>(box, ColliderType::Box, BodyState::Dynamic, glm::vec3(1.0f, 1.0f, 1.0f));

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(10.0f, 1.0f, 10.0f));
    scene.AssignParam<MeshRenderer>(ground, boxModel, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), groundTex);
    scene.AssignParam<RigidBody3D>(ground, ColliderType::Box, BodyState::Static, glm::vec3(10.0f, 1.0f, 10.0f));

    EntityID light2 = scene.AddEntity();
    scene.AssignParam<Light>(light2, glm::vec3(0.0f, 10.0f, 0.0f), 30.0f, 0.1f, 1.5f,
                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    EntityID sprite = scene.AddEntity();
    scene.AssignParam<Transform>(sprite, glm::vec3(2.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(sprite, slugariusTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), "what da dog doin",
                                      false, true);

    EntityID par = scene.AddEntity();
    scene.AssignParam<ParticleSystem>(par, slugariusTex, glm::vec3(2.0f, 3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::vec3(0.5f, 0.5f, 0.5f), -0.25f, glm::vec3(0.0f, 0.0f, 1.5f),
                                      glm::vec3(0.0f, 0.0f, 0.4f), 13.0f, 0.1f, -1.0f, glm::vec4(1.0f), glm::vec4(0.0f),
                                      glm::vec3(0.0f, -0.015f, 0.0f), 1.0f, 0.1f, 0.0f, 80.0f, 50, true, true);

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init();

    StartStartSystems();

    physicsSystem.OptimizeBroadPhase();

    MyContactListener* myListener = new MyContactListener();
    physicsSystem.SetContactListener(myListener);

    ImGuiLayer::Init();

    MyDebugRenderer debugRenderer;

    const JPH::BodyDrawFilter* filter = nullptr;

    JPH::BodyManager::DrawSettings settings;
    settings.mDrawShape = false;
    settings.mDrawBoundingBox = true;
    settings.mDrawShapeWireframe = true;

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        ImGuiLayer::NewFrame();

        UpdateSystems();

        physTimer -= engineState.deltaTime;

        // physicsSystem.DrawBodies(settings, &debugRenderer, filter);

        ImGuiLayer::EndFrame();
        window.Update();

        if (physTimer <= 0)
        {
            StepPhysics(engineState.deltaTime);
        }

        if (Input::GetKeyDown(Key::R))
        {
            engineState.scene.Get<ParticleSystem>(par)->playing = true;
            engineState.scene.Get<ParticleSystem>(par)->currentDuration = 0.0f;
        }
    }

    DestroyPhysics();
    ImGuiLayer::Terminate();

    return 0;
}
