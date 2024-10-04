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
    Model tileModel("res/models/tile.obj", false, false);
    unsigned int bottomTex = Utils::LoadTexture("res/models/textures/GROUND.png");
    unsigned int tex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");

    Scene scene;
    EntityID player = scene.AddEntity();
    scene.AssignParam<Transform>(player, glm::vec3(0.0f, 30.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(player, capsuleModel, glm::vec4(1.0f), tex);
    scene.AssignParam<RigidBody3D>(player, ColliderType::Capsule, BodyState::Dynamic, 1.0f, 2.0f);
    scene.AssignParam<PlayerMovement>(player, 0.2f, 0.6f);

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0001f, 0.0f), glm::vec3(3.0f, 1.0f, 3.0f));
    scene.AssignParam<MeshRenderer>(ground, tileModel, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), bottomTex);
    scene.AssignParam<RigidBody3D>(ground, ColliderType::Box, BodyState::Static, glm::vec3(60.0f, 1.0f, 60.0f));

    EntityID shotgun = scene.AddEntity();
    scene.AssignParam<Transform>(shotgun, glm::vec3(-6.0f, -1.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.4f, 0.4f, 0.4f));
    scene.AssignParam<MeshRenderer>(shotgun, gunModel, glm::vec4(1.0f), groundTex);
    scene.AssignParam<Gun>(shotgun, 1.1f, 0.47f, -0.67f);

    EntityID light2 = scene.AddEntity();
    scene.AssignParam<Light>(light2, glm::vec3(0.0f, 15.0f, 0.0f), 140.0f, 0.1f, 2.8f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
 
    EntityID playerLight = scene.AddEntity();
    scene.AssignParam<Light>(playerLight, glm::vec3(30.0f, 2.0f, 0.0f), 35.0f, 0.1f, 1.8f, glm::vec4(1.0f, 0.647f, 0.0f, 1.0f), false);  

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
        Renderer::lights[1].position = scene.Get<Transform>(player)->position;

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
