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

    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");
    unsigned int slugariusTex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int lineTex = Utils::LoadTexture("res/textures/Line.png");

    Scene scene;

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(10.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(ground, groundTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "what da dog doin", false, false);
    scene.AssignParam<sm2d::Rigidbody>(ground, sm2d::BodyType::sm2d_Static,
                                       scene.Get<Transform>(ground), 10.0f, true, 0.7f, 0.7f);
    scene.AssignParam<sm2d::Collider>(ground, sm2d::ColliderType::sm2d_AABB,
                                      sm2d::ColAABB(glm::vec2(5.0f, 0.5f)),
                                      scene.Get<sm2d::Rigidbody>(ground));

    EntityID ground2 = scene.AddEntity();
    scene.AssignParam<Transform>(ground2, glm::vec3(15.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(2.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(ground2, groundTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "what da dog doin", false, false);
    scene.AssignParam<sm2d::Rigidbody>(ground2, sm2d::BodyType::sm2d_Static,
                                       scene.Get<Transform>(ground2), 10.0f, true, 0.7f, 0.7f);
    scene.AssignParam<sm2d::Collider>(ground2, sm2d::ColliderType::sm2d_AABB,
                                      sm2d::ColAABB(glm::vec2(1.0f, 0.5f)),
                                      scene.Get<sm2d::Rigidbody>(ground2));

    EntityID sprite = scene.AddEntity();
    scene.AssignParam<Transform>(sprite, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(sprite, slugariusTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "what da dog doin", false, false);
    scene.AssignParam<sm2d::Rigidbody>(sprite, sm2d::BodyType::sm2d_Dynamic,
                                       scene.Get<Transform>(sprite), 2.0f, true, 0.7f, 0.7f);
    scene.AssignParam<sm2d::Collider>(sprite, sm2d::ColliderType::sm2d_AABB,
                                      sm2d::ColAABB(glm::vec2(0.5f, 0.5f)),
                                      scene.Get<sm2d::Rigidbody>(sprite));

    // EntityID par = scene.AddEntity();
    // scene.AssignParam<ParticleSystem>(par, lineTex, glm::vec3(2.0f, 3.0f, 0.0f),
    // glm::vec3(0.0f, 0.0f, 3.0f),
    //                                   glm::vec3(0.5f, 0.5f, 0.5f), -0.25f,
    //                                   glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f,
    //                                   0.8f, 0.4f), 7.0f, glm::vec3(2.0f, 0.0f, 2.0f),
    //                                   -1.0f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f),
    //                                   glm::vec4(0.0f, 0.5f, 0.0f, -0.4f),
    //                                   glm::vec3(0.0f, -0.035f, 0.0f), 2.0f, 0.1f, 0.0f,
    //                                   200.0f, 500, true, true, true);

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(false);

    StartStartSystems();

    ImGuiLayer::Init();

    sm2d::Collider* col1 = engineState.scene.Get<sm2d::Collider>(ground);
    sm2d::Collider* col2 = engineState.scene.Get<sm2d::Collider>(sprite);
    sm2d::Collider* col3 = engineState.scene.Get<sm2d::Collider>(ground2);

    sm2d::InsertLeaf(sm2d::bvh, col1, sm2d::ColAABBToABBB(*col1));
    sm2d::InsertLeaf(sm2d::bvh, col2, sm2d::ColAABBToABBB(*col2));
    sm2d::InsertLeaf(sm2d::bvh, col3, sm2d::ColAABBToABBB(*col3));

    // Main loop
    // -----------
    while (!window.ShouldClose())
    {
        // Start of frame
        ImGuiLayer::NewFrame();
        UpdateSystems();

        // Main loop logic
        // ---
        if (Input::GetKeyDown(Key::R))
        {
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->force.y += 500.0f;
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->force.x += 100.0f;
        }

        if (Input::GetKeyDown(Key::F))
        {
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->torque += 500.0f;
        }

        auto data = sm2d::TestColAABB(*col1, *col2);
        if (data)
        {
            col2->body->force.y += 20.0f;
        }

        for (auto& node : sm2d::bvh.nodes)
        {
            if (node.index == -1)
                continue;

            glm::vec3 topRight = glm::vec3(node.box.upperBound, 0.0f);
            glm::vec3 bottomLeft = glm::vec3(node.box.lowerBound, 0.0f);
            glm::vec3 bottomRight = glm::vec3(glm::vec2(topRight.x, bottomLeft.y), 0.0f);
            glm::vec3 topLeft = glm::vec3(glm::vec2(bottomLeft.x, topRight.y), 0.0f);
            std::vector<glm::vec3> points = {bottomLeft, topLeft, topRight, bottomRight};
            Renderer::RenderLine(
                points, engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio()),
                engineState.camera->GetViewMatrix());
        }

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    DestroyPhysics();
    ImGuiLayer::Terminate();

    return 0;
}
