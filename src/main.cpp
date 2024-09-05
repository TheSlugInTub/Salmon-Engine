#include <salmon.h>
#include <iostream>

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

    Model ourModel("res/models/Box.obj");
    unsigned int tex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");

    Scene scene;
    EntityID ent = scene.AddEntity();
    scene.AssignParam<Transform>(ent, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(ent, ourModel, glm::vec4(1.0f), tex);

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    scene.AssignParam<MeshRenderer>(ground, ourModel, glm::vec4(1.0f), groundTex);

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init();

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ClearScreen();

        // Do stuff here
        if (Input::GetKey(Key::Up))
        {
            scene.Get<Transform>(ent)->position.x += 0.016f;
        }

        UpdateSystems();
        window.Update();
    }

    return 0;
}
