#include <salmon/salmon.h>
#include <salmon/tilemap.h>
#include <salmon/physics_2d.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

std::string FPS;
auto        lastTime = std::chrono::high_resolution_clock::now();
int         frameCount = 0;
float       fps = 0.0f;

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false, true);
    // glfwSwapInterval(1);

    Scene scene;

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(false);
    Renderer::InitShaders();
    Renderer::Init2D();
    Renderer::InitTilemaps();

    InitPhysics2D();

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
            StepPhysics2D();
        }

        // Main loop logic
        // ---

        DrawHierarchy();
        DrawInspector();
        DrawTray();

        colResults.clear();
        sm2d::GetCollisionsInTree(sm2d::bvh, colResults);
        sm2d::ResolveCollisions(sm2d::bvh, colResults);
        colResults.clear();
        sm2d::GetCollisionsInTree(sm2d::bvh, colResults);
        sm2d::ResolveCollisions(sm2d::bvh, colResults);
        
        // Update FPS every second
        auto currentTime = std::chrono::high_resolution_clock::now();

        std::chrono::duration<float> elapsed = currentTime - lastTime;
        lastTime = currentTime;
        frameCount++;
        static float timeAccumulator = 0.0f;
        timeAccumulator += elapsed.count();
        if (timeAccumulator >= 0.1f)
        {
            fps = frameCount / timeAccumulator;

            // Reset counters
            frameCount = 0;
            timeAccumulator = 0.0f;

            // Update the FPS string
            FPS = std::to_string(fps);

            engineState.window->SetTitle(FPS.c_str());
        }

        // for (auto& node : sm2d::bvh.nodes)
        // {
        //     if (node.index == -1)
        //         continue;

        //     glm::vec3 topRight = glm::vec3(node.box.upperBound, 0.0f);
        //     glm::vec3 bottomLeft = glm::vec3(node.box.lowerBound, 0.0f);
        //     glm::vec3 bottomRight = glm::vec3(glm::vec2(topRight.x, bottomLeft.y), 0.0f);
        //     glm::vec3 topLeft = glm::vec3(glm::vec2(bottomLeft.x, topRight.y), 0.0f);
        //     std::vector<glm::vec3> points = {bottomLeft, topLeft, topRight, bottomRight};
        //     Renderer::RenderLine(
        //         points, engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio()),
        //         engineState.camera->GetViewMatrix());
        // }

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    DestroyPhysics2D();
    ImGuiLayer::Terminate();

    return 0;
}
