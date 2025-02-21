#include "salmon/shader.h"
#include "salmon/utils.h"
#include <salmon/salmon.h>
#include <salmon/tilemap.h>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

std::vector<std::pair<std::string, unsigned int>> tiles;
std::vector<std::string>                          tileNames;

Tilemap     tilemap(glm::vec2(1.0f, 1.0f));
inline int  selectedTileIndex = -1;
inline char buffer[128];
inline int  layer;

void DrawTilesMenu()
{
    ImGui::Begin("Tiles");

    for (int i = 0; i < 2; i++)
    {
        strncpy_s(buffer, tiles[i].first.c_str(), sizeof(buffer));
        if (ImGui::InputText(tileNames[i].c_str(), buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            tiles[i].first = std::string(buffer);
            tiles[i].second = Utils::LoadTexture(tiles[i].first.c_str());
        }
        if (tiles[i].second != 2000)
        {
            if (ImGui::ImageButton((ImTextureID)(intptr_t)tiles[i].second,
                                   ImVec2(64, 64)))
            {
                selectedTileIndex = (int)i;
            }
        }
    }

    ImGui::End();
}

void DrawTileTray()
{
    ImGui::Begin("Tray");

    ImGui::InputInt("Layer", &layer);

    ImGui::End();
}

void           TilemapDraw(Tilemap* tilemap, int layer);
nlohmann::json TilemapSave(Tilemap* tilemap);
void           TilemapLoad(Tilemap* tilemap, const nlohmann::json& j);

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false, true);
    // glfwSwapInterval(1);

    unsigned int lineTex = Utils::LoadTexture("res/textures/Line.png");
    unsigned int slugTex = Utils::LoadTexture("res/textures/Slugarius.png");

    tileNames.resize(100, "");
    tiles.resize(100, std::pair<std::string, unsigned int>("", 2000));

    tileNames[0] = "Wall Tile";
    tileNames[1] = "Default Wall Tile";

    tilemap.editorTiles.push_back(
        Tile(Utils::LoadTexture("res/textures/black.png"), "res/textures/black.png"));

    Scene scene;

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(true);
    Renderer::InitShaders();
    Renderer::Init2D();
    Renderer::InitTilemaps();

    StartEditorStartSystems();
    StartStartSystems();

    ImGuiLayer::Init();
    ImGuiLayer::EmbraceTheDarkness();

    // Main loop
    // -----------
    while (!window.ShouldClose())
    {
        // Start of frame
        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        ImGuiLayer::NewFrame();

        UpdateEditorSystems();
        UpdateSystems();

        DrawTilesMenu();
        DrawTileTray();
        TilemapDraw(&tilemap, layer);

        // Main loop logic
        // ---

        Renderer::RenderTilemap(tilemap, engineState.projMat, engineState.camera->GetViewMatrix());

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    ImGuiLayer::Terminate();

    return 0;
}
