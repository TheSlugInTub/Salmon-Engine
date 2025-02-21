#include "imgui/imgui.h"
#include "salmon/utils.h"
#include "salmon/window.h"
#include <salmon/salmon.h>
#include <salmon/tilemap.h>
#include <filesystem>
#include <fstream>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

struct FBOTexture
{
    FBOTexture(int width, int height)
    {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Bind() { glBindFramebuffer(GL_FRAMEBUFFER, FBO); }

    void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    void Rescale(int width, int height)
    {
        glDeleteTextures(1, &texture);

        // Create color attachment texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Bind framebuffer and attach textures
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    }

    unsigned int texture;
    unsigned int FBO;
};

std::vector<std::pair<std::string, unsigned int>> tiles;
std::vector<std::string>                          tileNames;

Tilemap     tilemap(glm::vec2(1.0f, 1.0f));
inline int  selectedTileIndex = -1;
inline char buffer[128];
inline char saveFile[128] = "heay.json";
inline int  layer;

void           TilemapDraw(Tilemap* tilemap, int layer);
nlohmann::json TilemapSave(Tilemap* tilemap);
void           TilemapLoad(Tilemap* tilemap, const nlohmann::json& j);

void SaveFile(const std::string& file)
{
    std::filesystem::path directory = "res/scenes";
    std::filesystem::create_directories(directory);
    std::filesystem::path filePath = directory / file;

    nlohmann::json j = {};

    j.push_back(TilemapSave(&tilemap));

    nlohmann::json tileJ = nlohmann::json::array();

    for (int i = 0; i < tiles.size(); ++i) { tileJ.push_back(tiles[i].first); }

    nlohmann::json tilePadding = {};
    tilePadding["tiles"] = tileJ;
    j.push_back(tilePadding);

    std::ofstream ffile(filePath);
    ffile << j.dump(4);
}

void LoadFile(const char* file)
{
    std::filesystem::path directory = "res/scenes";
    std::filesystem::path filePath = directory / file;

    std::ifstream ffile(filePath);
    if (!ffile.is_open())
    {
        std::cerr << "Failed to open file: " << file << '\n';
        return;
    }

    nlohmann::json j;
    ffile >> j;

    int i = 0;

    tilemap.editorTiles.clear();
    tilemap.tileTransforms.clear();
    tilemap.tileLayers.clear();
    tilemap.tileTextureIndices.clear();
    tiles.clear();

    for (nlohmann::json& tileJ : j)
    {
        if (i == 0)
        {
            TilemapLoad(&tilemap, tileJ);

            i++;
        }
        else
        {
            for (const std::string& tileJson : tileJ["tiles"])
            {
                tiles.push_back(std::pair<std::string, unsigned int>(
                    tileJson, Utils::LoadTexture(tileJson.c_str())));
            }
        }
    }

    ffile.close();
}

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
            if (ImGui::ImageButton((ImTextureID)(intptr_t)tiles[i].second, ImVec2(64, 64)))
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
    ImGui::InputText("Save File", saveFile, sizeof(saveFile), ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("Save"))
    {
        SaveFile(saveFile);
    }
    if (ImGui::Button("Load"))
    {
        LoadFile(saveFile);
    }

    ImGui::End();
}

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

        // ImGui stuff
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
