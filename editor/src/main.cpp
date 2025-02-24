#include "salmon/utils.h"
#include <salmon/salmon.h>
#include <salmon/tilemap.h>
#include <filesystem>
#include <fstream>
#include <salmon/stb_image_write.h>
#include <thread>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f,
              0.0f, 90.0f);

int levelWidth = 1280;
int levelHeight = 960;

std::vector<glm::vec3> screenPoints = {
    glm::vec3(levelWidth / 16 / 2, levelHeight / 16 / 2, 0.0f),
    glm::vec3(-levelWidth / 16 / 2, levelHeight / 16 / 2, 0.0f),
    glm::vec3(-levelWidth / 16 / 2, -levelHeight / 16 / 2, 0.0f),
    glm::vec3(levelWidth / 16 / 2, -levelHeight / 16 / 2, 0.0f)};

struct FBOTexture
{
    FBOTexture(int width, int height)
    {
        glGenFramebuffers(1, &FBO);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture, 0);

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Bind framebuffer and attach textures
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture, 0);
    }

    unsigned int texture;
    unsigned int FBO;
};

struct PrefTile
{
    std::string    texturePath = {};
    unsigned int   texture = 0;
    glm::vec2      dimensions = {};
    unsigned char* data = nullptr;

    PrefTile(const std::string& path) : texturePath(path)
    {
        glGenTextures(1, &texture);

        int width, height, nrComponents;
        data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                         GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            dimensions = glm::vec2(width, height);

            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << '\n';
        }
    }
};

std::vector<PrefTile>    tiles;
std::vector<std::string> tileNames;

Tilemap     tilemap(glm::vec2(1.0f, 1.0f));
inline int  selectedTileIndex = -1;
inline char buffer[128];
inline char saveFile[128] = "heay.json";
inline char renderFile[128] = "stockade.png";
inline int  layer;

void           TilemapDraw(Tilemap* tilemap, int layer);
nlohmann::json TilemapSave(Tilemap* tilemap);
void           TilemapLoad(Tilemap* tilemap, const nlohmann::json& j);

void RenderScene();

void SaveFile(const std::string& file)
{
    std::filesystem::path directory = "res/scenes";
    std::filesystem::create_directories(directory);
    std::filesystem::path filePath = directory / file;

    nlohmann::json j = {};

    j.push_back(TilemapSave(&tilemap));

    nlohmann::json tileJ = nlohmann::json::array();

    for (int i = 0; i < tiles.size(); ++i)
    {
        tileJ.push_back(tiles[i].texturePath);
    }

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
                tiles.push_back(PrefTile(tileJson));
            }
        }
    }

    ffile.close();
}

void DrawTilesMenu()
{
    ImGui::Begin("Tiles");

    for (int i = 0; i < 13; i++)
    {
        strncpy_s(buffer, tiles[i].texturePath.c_str(), sizeof(buffer));
        if (ImGui::InputText(tileNames[i].c_str(), buffer, sizeof(buffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            tiles[i].texturePath = std::string(buffer);
            tiles[i].texture = Utils::LoadTexture(tiles[i].texturePath.c_str());
        }
        if (tiles[i].texture != 2000)
        {
            if (ImGui::ImageButton((ImTextureID)(intptr_t)tiles[i].texture,
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
    ImGui::InputText("Save File", saveFile, sizeof(saveFile),
                     ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("Save"))
    {
        SaveFile(saveFile);
    }
    if (ImGui::Button("Load"))
    {
        LoadFile(saveFile);
    }
    if (ImGui::DragInt("Width", &levelWidth) ||
        ImGui::DragInt("Height", &levelHeight))
    {
        screenPoints = {
            glm::vec3(levelWidth / 16 / 2, levelHeight / 16 / 2, 0.0f),
            glm::vec3(-levelWidth / 16 / 2, levelHeight / 16 / 2, 0.0f),
            glm::vec3(-levelWidth / 16 / 2, -levelHeight / 16 / 2, 0.0f),
            glm::vec3(levelWidth / 16 / 2, -levelHeight / 16 / 2, 0.0f)};
    }
    ImGui::InputText("Render File", renderFile, sizeof(renderFile),
                     ImGuiInputTextFlags_EnterReturnsTrue);
    if (ImGui::Button("Render"))
    {
        RenderScene();
    }

    ImGui::End();
}

#define DIR_UP                0
#define DIR_DOWN              1
#define DIR_LEFT              2
#define DIR_RIGHT             3
#define DIR_UP_LEFT           4
#define DIR_UP_RIGHT          5
#define DIR_DOWN_LEFT         6
#define DIR_DOWN_RIGHT        7
#define DIR_UP_LEFT_CORNER    8
#define DIR_UP_RIGHT_CORNER   9
#define DIR_DOWN_LEFT_CORNER  10
#define DIR_DOWN_RIGHT_CORNER 11

int GetWallTileTextureIndex(const unsigned char* directions) {
    // Check for corner cases first (most specific)
    if (directions[DIR_UP] && directions[DIR_RIGHT] && !directions[DIR_UP_RIGHT])
        return DIR_UP_RIGHT_CORNER;  // NURC
    
    if (directions[DIR_UP] && directions[DIR_LEFT] && !directions[DIR_UP_LEFT])
        return DIR_UP_LEFT_CORNER;   // NULC
    
    if (directions[DIR_DOWN] && directions[DIR_RIGHT] && !directions[DIR_DOWN_RIGHT])
        return DIR_DOWN_RIGHT_CORNER; // NDRC
    
    if (directions[DIR_DOWN] && directions[DIR_LEFT] && !directions[DIR_DOWN_LEFT])
        return DIR_DOWN_LEFT_CORNER;  // NDLC
    
    // Check for two-direction cases
    if (directions[DIR_DOWN] && directions[DIR_RIGHT])
        return DIR_DOWN_RIGHT;        // NDR
    
    if (directions[DIR_DOWN] && directions[DIR_LEFT])
        return DIR_DOWN_LEFT;         // NDL
    
    if (directions[DIR_UP] && directions[DIR_RIGHT])
        return DIR_UP_RIGHT;          // NUR
    
    if (directions[DIR_UP] && directions[DIR_LEFT])
        return DIR_UP_LEFT;           // NUL
    
    // Check for single-direction cases
    if (directions[DIR_UP])
        return DIR_UP;                // NU
    
    if (directions[DIR_DOWN])
        return DIR_DOWN;              // ND
    
    if (directions[DIR_LEFT])
        return DIR_LEFT;              // NL
    
    if (directions[DIR_RIGHT])
        return DIR_RIGHT;             // NR
    
    return 12;  // Default tile index
}

void RenderScene()
{
    const int    CHANNELS = 4;
    const size_t outputSize = levelWidth * levelHeight * CHANNELS;

    // Create output buffer initialized to transparent
    std::vector<unsigned char> outputBuffer;
    outputBuffer.resize(outputSize, 0);

    // Find the bounds of all tiles to calculate offset
    glm::vec2 minPos(std::numeric_limits<float>::max());
    for (int i = 0; i < tilemap.tileTextureIndices.size(); ++i)
    {
        glm::vec2 tilePos = Utils::GetPositionOfMat4(tilemap.tileTransforms[i]);
        minPos.x = std::min(minPos.x, tilePos.x);
        minPos.y = std::min(minPos.y, tilePos.y);
    }

    // For each tile in the level
    for (int i = 0; i < tilemap.tileTextureIndices.size(); ++i)
    {
        // Get the corresponding prefab tile
        PrefTile* pref = nullptr;

        glm::vec2 tilePos = Utils::GetPositionOfMat4(tilemap.tileTransforms[i]);
        // Offset position so minimum coordinate is at 0,0
        tilePos.x -= minPos.x;
        tilePos.y -= minPos.y;

        int tileTexture = (int)tilemap.tileTextureIndices[i];

        switch (tileTexture)
        {
            case 0: // Terrain tile
            {
                // up, down, left, right
                // up left corner, up right cornver, down
                // left corner, down right corner

                unsigned char directions[8] = {
                    false, false, false, false, false, false, false, false,
                };

                for (int j = 0; j < tilemap.tileTextureIndices.size(); ++j)
                {
                    glm::vec2 jTilePos = glm::vec2(
                        Utils::GetPositionOfMat4(tilemap.tileTransforms[i]));

                    if (jTilePos == (tilePos + glm::vec2(1.0f, 0.0f)))
                    {
                        directions[DIR_RIGHT] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(-1.0f, 0.0f)))
                    {
                        directions[DIR_LEFT] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(0.0f, 1.0f)))
                    {
                        directions[DIR_UP] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(0.0f, -1.0f)))
                    {
                        directions[DIR_DOWN] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(1.0f, 1.0f)))
                    {
                        directions[DIR_UP_RIGHT] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(1.0f, -1.0f)))
                    {
                        directions[DIR_DOWN_RIGHT] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(-1.0f, 1.0f)))
                    {
                        directions[DIR_UP_LEFT] = true;
                    }
                    if (jTilePos == (tilePos + glm::vec2(-1.0f, -1.0f)))
                    {
                        directions[DIR_DOWN_LEFT] = true;
                    }

                    pref = &tiles[GetWallTileTextureIndex(directions)];
                }

                break;
            }
            case 1:
            {
                break;
            }
            case 2:
            {
                break;
            }
            case 3:
            {
                break;
            }
        }

        // Calculate pixel positions
        int tilePixelX = static_cast<int>(tilePos.x * pref->dimensions.x);
        int tilePixelY = static_cast<int>(tilePos.y * pref->dimensions.y);

        // Copy tile data to output buffer
        for (int y = 0; y < static_cast<int>(pref->dimensions.y); y++)
        {
            for (int x = 0; x < static_cast<int>(pref->dimensions.x); x++)
            {
                // Calculate source and destination positions
                int srcPos =
                    (y * static_cast<int>(pref->dimensions.x) + x) * CHANNELS;
                int destX = tilePixelX + x;
                int destY = tilePixelY + y;

                // Skip if outside bounds
                if (destX < 0 || destX >= levelWidth || destY < 0 ||
                    destY >= levelHeight)
                    continue;

                int destPos = (destY * levelWidth + destX) * CHANNELS;

                // Copy RGBA values
                for (int c = 0; c < CHANNELS; c++)
                {
                    outputBuffer[destPos + c] = pref->data[srcPos + c];
                }
            }
        }
    }

    stbi_flip_vertically_on_write(true);

    // Write the final image to PNG
    auto err = stbi_write_png(renderFile, levelWidth, levelHeight, CHANNELS,
                              outputBuffer.data(), levelWidth * CHANNELS);

    if (!err)
        std::cout << "You done messed big time bruv!\n";
}

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false, true);
    // glfwSwapInterval(1);

    unsigned int lineTex = Utils::LoadTexture("res/textures/Line.png");
    unsigned int slugTex = Utils::LoadTexture("res/textures/Slugarius.png");

    tileNames.resize(100, "");
    PrefTile defaultTile("");
    defaultTile.texture = 2000;
    tiles.resize(100, defaultTile);

    tileNames[0] = "Wall Tile No Up Tile";
    tileNames[1] = "Wall Tile No Down Tile";
    tileNames[2] = "Wall Tile No Left Tile";
    tileNames[3] = "Wall Tile No Right Tile";
    tileNames[4] = "Wall Tile No Up Left Tile";
    tileNames[5] = "Wall Tile No Up Right Tile";
    tileNames[6] = "Wall Tile No Down Left Tile";
    tileNames[7] = "Wall Tile No Down Right Tile";
    tileNames[8] = "Wall Tile No Up Left Corner Tile";
    tileNames[9] = "Wall Tile No Up Right Corner Tile";
    tileNames[10] = "Wall Tile No Down Left Corner Tile";
    tileNames[11] = "Wall Tile No Down Right Corner Tile";
    tileNames[12] = "Default Wall Tile";

    tilemap.editorTiles.push_back(
        Tile(Utils::LoadTexture("res/textures/black.png"),
             "res/textures/black.png"));

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

        Renderer::RenderTilemap(tilemap, engineState.projMat,
                                engineState.camera->GetViewMatrix());
        Renderer::RenderLine(screenPoints, engineState.projMat,
                             engineState.camera->GetViewMatrix(),
                             glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    ImGuiLayer::Terminate();

    return 0;
}
