#include <iostream>
#include <salmon.h>
#include <string>
#include <syslocked/components.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

float physTimer = 3.0f;

int main()
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false);
    glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSwapInterval(1);

    StartPhysics();

    Model boxModel("res/models/Box.obj");
    Model capsuleModel("res/models/Capsule.obj");
    Model gunModel("res/models/Shotgun.dae");
    Model strayModel("res/models/stray/StrayRun.glb");
    Model tileModel("res/models/tile.obj", false, false);
    Model bulletModel("res/models/Bullet.obj", false, false);
    unsigned int bottomTex = Utils::LoadTexture("res/models/textures/GROUND.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");
    unsigned int bulletTex = Utils::LoadTexture("res/models/textures/Bullet.png");

    Scene scene;
    EntityID player = scene.AddEntity();
    scene.AssignParam<Transform>(player, glm::vec3(0.0f, 10.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<RigidBody3D>(player, ColliderType::Capsule, BodyState::Dynamic, 1.0f, 2.0f, RigidbodyID_Player);
    scene.AssignParam<PlayerMovement>(player, 0.34f, 0.6f);
    scene.AssignParam<Health>(player, 3);

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0001f, 0.0f),
                                 glm::vec3(3.0f, 1.0f, 3.0f));
    scene.AssignParam<MeshRenderer>(ground, tileModel, glm::vec4(1.0f, 0.0f, 1.0f, 1.0f), bottomTex);
    scene.AssignParam<RigidBody3D>(ground, ColliderType::Box, BodyState::Static, glm::vec3(74.0f, 0.85f, 74.0f));

    Animation gunShootAnim("res/models/ShotgunShoot.dae", &gunModel);
    Animation strayRunAnim("res/models/stray/StrayRun.glb", &strayModel);

    EntityID shotgun = scene.AddEntity();
    scene.AssignParam<Transform>(shotgun, glm::vec3(-6.0f, -1.0f, 0.0f), glm::vec3(0.0f, 2.0f, 0.0f),
                                 glm::vec3(0.4f, 0.4f, 0.4f));
    scene.AssignParam<MeshRenderer>(shotgun, gunModel, glm::vec4(1.0f), groundTex);
    scene.AssignParam<Gun>(shotgun, 1.8f, 0.5f, -0.87f, &gunShootAnim, 1.0f, bulletModel, bulletTex, 99000.0f);
    scene.AssignParam<Animator>(shotgun, &gunShootAnim, true, false, 1.4f);

    EntityID light2 = scene.AddEntity();
    scene.AssignParam<Light>(light2, glm::vec3(0.0f, 15.0f, 0.0f), 140.0f, 0.1f, 2.8f,
                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    EntityID playerLight = scene.AddEntity();
    scene.AssignParam<Light>(playerLight, glm::vec3(30.0f, 2.0f, 0.0f), 35.0f, 0.1f, 1.8f,
                             glm::vec4(1.0f, 0.647f, 0.0f, 1.0f), false);

    EntityID enemy = scene.AddEntity();
    scene.AssignParam<Transform>(enemy, glm::vec3(2.0f, -5.0f, 0.0f), glm::vec3(2.5f, 0.0f, 0.0f),
                                 glm::vec3(3.03f, 3.03f, 3.03f));
    scene.AssignParam<MeshRenderer>(enemy, strayModel, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), bottomTex);
    scene.AssignParam<RigidBody3D>(enemy, ColliderType::Capsule, BodyState::Dynamic, 1.0f, 4.0f, RigidbodyID_Enemy,
                                   glm::vec3(0.0f, -3.4f, 0.0f));
    scene.AssignParam<Enemy>(enemy, 15);
    scene.AssignParam<Stray>(enemy, 799110.0f, 10000000.0f, scene.Get<Transform>(player));
    scene.AssignParam<Animator>(enemy, &strayRunAnim, true, true, 1.7f);

    EntityID enemySpawner = scene.AddEntity();
    scene.AssignParam<EnemySpawner>(enemySpawner, 3.0f, scene.Get<Transform>(ground), scene.Get<Transform>(player),
                                    &strayModel, &strayRunAnim);

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
        glm::vec3 cameraPos = scene.Get<Transform>(player)->position;
        cameraPos.y += 1.7f;
        camera.Position = cameraPos;
        Renderer::lights[1].position = scene.Get<Transform>(player)->position;

        //physicsSystem.DrawBodies(settings, &debugRenderer, filter);

        ImGuiLayer::EndFrame();
        window.Update();

        if (physTimer <= 0)
        {
            StepPhysics(engineState.deltaTime);
        }
    }

    DestroyPhysics();
    ImGuiLayer::Terminate();

    return 0;
}
