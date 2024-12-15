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
    // glfwSwapInterval(1);

    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");
    unsigned int slugariusTex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int lineTex = Utils::LoadTexture("res/textures/Line.png");
    unsigned int circleTex = Utils::LoadTexture("res/textures/SlugariusCircle.png");

    Scene scene;

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(10.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(ground, groundTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Ground", false, false);
    scene.AssignParam<sm2d::Rigidbody>(ground, sm2d::BodyType::sm2d_Static,
                                       scene.Get<Transform>(ground), 50.0f, true, 0.7f, 0.7f);
    scene.AssignParam<sm2d::Collider>(ground, sm2d::ColliderType::sm2d_AABB,
                                      sm2d::ColAABB(glm::vec2(5.0f, 0.5f)),
                                      scene.Get<sm2d::Rigidbody>(ground));

    EntityID ground2 = scene.AddEntity();
    scene.AssignParam<Transform>(ground2, glm::vec3(15.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(2.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(ground2, groundTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Ground2", false, false);
    scene.AssignParam<sm2d::Rigidbody>(ground2, sm2d::BodyType::sm2d_Static,
                                       scene.Get<Transform>(ground2), 50.0f, true, 0.7f, 0.7f);
    scene.AssignParam<sm2d::Collider>(ground2, sm2d::ColliderType::sm2d_AABB,
                                      sm2d::ColAABB(glm::vec2(1.0f, 0.5f)),
                                      scene.Get<sm2d::Rigidbody>(ground2));

    EntityID sprite = scene.AddEntity();
    scene.AssignParam<Transform>(sprite, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(sprite, slugariusTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Sprite", false, false);
    scene.AssignParam<sm2d::Rigidbody>(sprite, sm2d::BodyType::sm2d_Dynamic,
                                       scene.Get<Transform>(sprite), 2.0f, true, 0.56f, 0.56f, 1.0f,
                                       false, 0.8f);
    scene.AssignParam<sm2d::Collider>(
        sprite, sm2d::ColliderType::sm2d_Polygon,
        sm2d::ColPolygon({glm::vec2(-0.5f, -0.5f), glm::vec2(-0.5f, 0.5f), glm::vec2(0.5f, 0.5f),
                          glm::vec2(0.5f, -0.5f)}),
        scene.Get<sm2d::Rigidbody>(sprite));

    // EntityID circle = scene.AddEntity();
    // scene.AssignParam<Transform>(circle, glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f,
    // 0.0f),
    //                              glm::vec3(1.0f, 1.0f, 1.0f));
    // scene.AssignParam<SpriteRenderer>(circle, circleTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    //                                   "Circle", false, false);
    // scene.AssignParam<sm2d::Rigidbody>(circle, sm2d::BodyType::sm2d_Dynamic,
    //                                    scene.Get<Transform>(circle), 2.0f, true, 0.56f,
    //                                    0.9f, 1.0f, false, -0.001f);
    // scene.AssignParam<sm2d::Collider>(circle, sm2d::ColliderType::sm2d_Circle,
    //                                   sm2d::ColCircle(0.5f), scene.Get<sm2d::Rigidbody>(circle));

    EntityID box = scene.AddEntity();
    scene.AssignParam<Transform>(box, glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<SpriteRenderer>(box, slugariusTex, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                                      "Circle", false, false);
    scene.AssignParam<sm2d::Rigidbody>(box, sm2d::BodyType::sm2d_Dynamic, scene.Get<Transform>(box),
                                       2.0f, true, 0.56f, 0.56f, 1.0f, false, 0.8f);
    scene.AssignParam<sm2d::Collider>(
        box, sm2d::ColliderType::sm2d_Polygon,
        sm2d::ColPolygon({glm::vec2(-0.5f, -0.5f), glm::vec2(-0.5f, 0.5f), glm::vec2(0.5f, 0.5f),
                          glm::vec2(0.5f, -0.5f)}),
        /*sm2d::ColAABB(glm::vec2(0.5f, 0.5f)),*/ scene.Get<sm2d::Rigidbody>(box));

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(false);

    StartStartSystems();

    ImGuiLayer::Init();

    sm2d::Collider* col1 = engineState.scene.Get<sm2d::Collider>(ground);
    sm2d::Collider* col2 = engineState.scene.Get<sm2d::Collider>(sprite);
    sm2d::Collider* col3 = engineState.scene.Get<sm2d::Collider>(ground2);
    sm2d::Collider* col4 = engineState.scene.Get<sm2d::Collider>(box);

    std::vector<sm2d::CollisionData> colResults;

    // Main loop
    // -----------
    while (!window.ShouldClose())
    {
        // Start of frame
        ImGuiLayer::NewFrame();
        UpdateSystems();

        // std::cout << "Is root node's child1 a leaf? " <<
        // sm2d::bvh.nodes[sm2d::bvh.nodes[sm2d::bvh.rootIndex].child2].leaf << '\n';

        // Main loop logic
        // ---
        if (Input::GetKey(Key::Left))
        {
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->force.x -= 10.0f;
        }
        if (Input::GetKey(Key::Right))
        {
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->force.x += 10.0f;
        }
        if (Input::GetKey(Key::Up))
        {
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->force.y += 10.0f;
        }
        if (Input::GetKey(Key::Down))
        {
            engineState.scene.Get<sm2d::Rigidbody>(sprite)->force.y -= 10.0f;
        }

        if (Input::GetKeyDown(Key::F))
        {
            engineState.scene.Get<sm2d::Rigidbody>(box)->torque += 500.0f;
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
