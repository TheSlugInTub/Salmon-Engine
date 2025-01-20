#include <salmon/salmon.h>
#include <salmon/tilemap.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false, true);
    // glfwSwapInterval(1);

    unsigned int lineTex = Utils::LoadTexture("res/textures/Line.png");
    unsigned int slugTex = Utils::LoadTexture("res/textures/Slugarius.png");

    Scene scene;

    EntityID tilemap = scene.AddEntity();
    scene.AssignParam<Tilemap>(tilemap, glm::vec2(1.0f, 1.0f));
    scene.AssignParam<Name>(tilemap, "Tilemap");

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(true);
    Renderer::InitShaders();
    Renderer::Init2D();
    Renderer::InitTilemaps();

    StartEditorStartSystems();

    ImGuiLayer::Init();
    ImGuiLayer::EmbraceTheDarkness();

    std::vector<sm2d::Manifold> colResults;

    // Main loop
    // -----------
    while (!window.ShouldClose())
    {
        // Start of frame
        ImGuiLayer::NewFrame();

        UpdateEditorSystems();
        if (playing)
        {
            UpdateSystems();
        }

        // Main loop logic
        // ---

        DrawHierarchy();
        DrawInspector();
        DrawTray();

        colResults.clear();
        sm2d::GetCollisionsInTree(sm2d::bvh, colResults);
        sm2d::ResolveCollisions(sm2d::bvh, colResults);

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    ImGuiLayer::Terminate();

    return 0;
}
