#include <salmon.h>
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

    StartPhysics();

    Model ourModel("res/models/Box.obj");
    Model capsuleModel("res/models/Capsule.obj");
    unsigned int tex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");

    Scene scene;
    EntityID ent = scene.AddEntity();
    scene.AssignParam<Transform>(ent, glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(ent, capsuleModel, glm::vec4(1.0f), tex);
    //scene.AssignParam<RigidBody3D>(ent, ColliderType::Box, BodyState::Dynamic, glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<RigidBody3D>(ent, ColliderType::Capsule, BodyState::Dynamic, 1.0f, 2.0f);

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    scene.AssignParam<MeshRenderer>(ground, ourModel, glm::vec4(1.0f), groundTex);
    scene.AssignParam<RigidBody3D>(ground, ColliderType::Box, BodyState::Static, glm::vec3(10.0f, 1.0f, 10.0f));

    EntityID light2 = scene.AddEntity();
    scene.AssignParam<Light>(light2, glm::vec3(0.0f, 5.0f, 0.0f), 25.0f, 0.1f, 1.0f, glm::vec4(1.0f));
 
    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init();

    StartStartSystems();

    physicsSystem.OptimizeBroadPhase();

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

	BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        auto box_id = scene.Get<RigidBody3D>(ent)->body->GetID();
        auto body = scene.Get<RigidBody3D>(ent)->body;

        if (Input::GetKey(Key::E))
        {
            Vec3 force(10000.0f, 50000.0f, 0.0f); 
            bodyInterface.AddForce(box_id, force * 2);
        }

        if (Input::GetKey(Key::R))
        {
            Vec3 force(0.0f, 50000.0f, 10000.0f); 
            bodyInterface.AddForce(box_id, force * 2);
        }

        UpdateSystems();

        MyDebugRenderer debugRenderer;
 
        const BodyDrawFilter *filter = nullptr; // Optional filter, can be null

        BodyManager::DrawSettings settings;
        settings.mDrawShape = false;
        settings.mDrawBoundingBox = true;
        settings.mDrawShapeWireframe = true;

        physicsSystem.DrawBodies(settings, &debugRenderer, filter);
        
        window.Update();

        StepPhysics(deltaTime);
    }

    DestroyPhysics();

    return 0;
}
