#include <salmon/salmon.h>
#include <glm/gtx/string_cast.hpp>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

float physTimer = 0.3f;

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT);
    // glfwSwapInterval(1);

    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");
    unsigned int slugariusTex = Utils::LoadTexture("res/textures/Slugarius.png");

    Scene scene;

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(10.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(ground, groundTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Ground");
    scene.AssignParam<sm2d::Rigidbody>(ground, sm2d::BodyType::sm2d_Static,
                                       scene.Get<Transform>(ground), 100.0f, false, 0.7f, 0.7f,
                                       0.0f, true, 1.5f);
    scene.AssignParam<sm2d::Collider>(
        ground, sm2d::ColliderType::sm2d_Polygon,
        sm2d::ColPolygon({glm::vec2(-5.0f, -0.5f), glm::vec2(-5.0f, 0.5f), glm::vec2(5.0f, 0.5f),
                          glm::vec2(5.0f, -0.5f)}),
        scene.Get<sm2d::Rigidbody>(ground));

    EntityID sprite = scene.AddEntity();
    scene.AssignParam<Transform>(sprite, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(sprite, slugariusTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Sprite");
    scene.AssignParam<sm2d::Rigidbody>(sprite, sm2d::BodyType::sm2d_Dynamic,
                                       scene.Get<Transform>(sprite), 2.0f, true, 0.56f, 0.56f, 0.0f,
                                       false, 0.4f);
    scene.AssignParam<sm2d::Collider>(
        sprite, sm2d::ColliderType::sm2d_Polygon,
        sm2d::ColPolygon({glm::vec2(-0.5f, -0.5f), glm::vec2(-0.5f, 0.5f), glm::vec2(0.5f, 0.5f),
                          glm::vec2(0.5f, -0.5f)}),
        scene.Get<sm2d::Rigidbody>(sprite));

    EntityID box = scene.AddEntity();
    scene.AssignParam<Transform>(box, glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 3.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(box, slugariusTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Circle");
    scene.AssignParam<sm2d::Rigidbody>(box, sm2d::BodyType::sm2d_Dynamic, scene.Get<Transform>(box),
                                       10.0f, true, 0.56f, 0.56f, 0.0f, false, 2.5f);
    scene.AssignParam<sm2d::Collider>(
        box, sm2d::ColliderType::sm2d_Polygon,
        sm2d::ColPolygon({glm::vec2(-0.5f, -1.5f), glm::vec2(-0.5f, 1.5f), glm::vec2(0.5f, 1.5f),
                          glm::vec2(0.5f, -1.5f)}),
        scene.Get<sm2d::Rigidbody>(box));

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(false, true);

    StartStartSystems();

    ImGuiLayer::Init();

    sm2d::Collider* col2 = engineState.scene.Get<sm2d::Collider>(sprite);

    std::vector<sm2d::Manifold> colResults;

    // Main loop
    // -----------
    while (!window.ShouldClose())
    {
        // Start of frame
        ImGuiLayer::NewFrame();
        UpdateSystems();

        // Main loop logic
        // ---
        if (Input::GetKey(Key::Left))
        {
            col2->body->hasMoved = true;
            col2->body->awake = true;
            col2->body->force.x -= 20.0f;
        }
        if (Input::GetKey(Key::Right))
        {
            col2->body->hasMoved = true;
            col2->body->awake = true;
            col2->body->force.x += 20.0f;
        }
        if (Input::GetKey(Key::Up))
        {
            col2->body->hasMoved = true;
            col2->body->awake = true;
            col2->body->force.y += 20.0f;
        }
        if (Input::GetKey(Key::Down))
        {
            col2->body->hasMoved = true;
            col2->body->awake = true;
            col2->body->force.y -= 20.0f;
        }

        colResults.clear();
        sm2d::GetCollisionsInTree(sm2d::bvh, colResults);
        sm2d::ResolveCollisions(sm2d::bvh, colResults);

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    DestroyPhysics();
    ImGuiLayer::Terminate();

    return 0;
}
