#include <salmon.h>
#include <syslocked/components.h>
#include <iostream>
#include <string>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false);
    glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    StartPhysics();

    Model ourModel("res/models/Box.obj");
    Model capsuleModel("res/models/Capsule.obj");
    Model gunModel("res/models/Shotgun.obj");
    unsigned int tex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");

    Scene scene;
    EntityID player = scene.AddEntity();
    scene.AssignParam<Transform>(player, glm::vec3(0.0f, 20.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(player, capsuleModel, glm::vec4(1.0f), tex);
    scene.AssignParam<RigidBody3D>(player, ColliderType::Capsule, BodyState::Dynamic, 1.0f, 2.0f);
    scene.AssignParam<PlayerMovement>(player, 0.2f, 0.6f);

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(50.0f, 1.0f, 50.0f));
    scene.AssignParam<MeshRenderer>(ground, ourModel, glm::vec4(1.0f), groundTex);
    scene.AssignParam<RigidBody3D>(ground, ColliderType::Box, BodyState::Static, glm::vec3(50.0f, 1.0f, 50.0f));

    EntityID shotgun = scene.AddEntity();
    scene.AssignParam<Transform>(shotgun, glm::vec3(-6.0f, -1.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(shotgun, gunModel, glm::vec4(1.0f), groundTex);
    scene.AssignParam<Gun>(shotgun, 5.0f, 2.0f, -1.5f);

    EntityID light2 = scene.AddEntity();
    scene.AssignParam<Light>(light2, glm::vec3(0.0f, 7.0f, 0.0f), 45.0f, 0.1f, 2.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    
    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init();

    StartStartSystems();

    physicsSystem.OptimizeBroadPhase();

    ImGuiLayer::Init();

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        // per-frame time logic
        // --------------------
        float currplayerFrame = static_cast<float>(glfwGetTime());
        deltaTime = currplayerFrame - lastFrame;
        lastFrame = currplayerFrame;

        ImGuiLayer::NewFrame();

        UpdateSystems();

        glm::vec3 cameraPos = scene.Get<Transform>(player)->position;
        cameraPos.y += 1.7f;
        camera.Position = cameraPos;

        MyDebugRenderer debugRenderer;
 
        const BodyDrawFilter *filter = nullptr; 

        BodyManager::DrawSettings settings;
        settings.mDrawShape = false;
        settings.mDrawBoundingBox = true;
        settings.mDrawShapeWireframe = true;

        //physicsSystem.DrawBodies(settings, &debugRenderer, filter);
 
        ImGuiLayer::EndFrame();
        window.Update();

        StepPhysics(deltaTime);
    }

    DestroyPhysics();
    ImGuiLayer::Terminate();

    return 0;
}
