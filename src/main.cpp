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
    unsigned int tex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");

    Scene scene;
    EntityID ent = scene.AddEntity();
    scene.AssignParam<Transform>(ent, glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(ent, ourModel, glm::vec4(1.0f), tex);
    scene.AssignParam<RigidBody3D>(ent, ColliderType::Box, BodyState::Dynamic, glm::vec3(0.7f, 0.7f, 0.7f));

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -3.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    scene.AssignParam<MeshRenderer>(ground, ourModel, glm::vec4(1.0f), groundTex);
    scene.AssignParam<RigidBody3D>(ground, ColliderType::Box, BodyState::Static, glm::vec3(10.0f, 1.0f, 10.0f));

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

        glClear(GL_DEPTH_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT);

	    BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        auto box_id = scene.Get<RigidBody3D>(ent)->body->GetID();

        if (Input::GetKey(Key::E))
        {
            Vec3 force(10000.0f, 50000.0f, 0.0f); 
            bodyInterface.AddForce(box_id, force);
        }

        if (Input::GetKey(Key::R))
        {
            Vec3 force(0.0f, 50000.0f, 10000.0f); 
            bodyInterface.AddForce(box_id, force);
        }

        MyDebugRenderer debugRenderer;

        UpdateSystems();

        debugRenderer.DrawLines();

		float aspectRatio = engineState.window->GetAspectRatio();
        glm::vec3 point1(0.0f, 0.0f, 0.0f);
        glm::vec3 point2(1.0f, 1.0f, 0.0f);
        glm::vec3 point3(-1.0f, 2.0f, 3.0f);

        Renderer::RenderLine(point1, point2, engineState.camera->GetProjMatrix(aspectRatio), engineState.camera->GetViewMatrix());
        Renderer::RenderLine(point2, point3, engineState.camera->GetProjMatrix(aspectRatio), engineState.camera->GetViewMatrix());
        
        window.Update();

        StepPhysics(deltaTime);
    }

    DestroyPhysics();

    return 0;
}
