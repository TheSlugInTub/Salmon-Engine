#include <salmon/editor.h>
#include <bombratter/background_sprite.h>
#include <salmon/components.h>
#include <glm/gtx/string_cast.hpp>
#include <sm2d/colliders.h>

void CalculateBackgroundScreenMinMax(
    glm::vec3 backgroundPosition, glm::vec3 backgroundScale,
    glm::mat4 viewMatrix, glm::mat4 projectionMatrix,
    glm::vec2 viewportSize, glm::vec2& minUV, glm::vec2& maxUV)
{
    // Transform the background's center position to clip space
    glm::vec4 clipSpacePos = projectionMatrix * viewMatrix *
                             glm::vec4(backgroundPosition, 1.0);

    // Perspective divide to get NDC (normalized device coordinates)
    glm::vec3 ndcPos = glm::vec3(clipSpacePos) / clipSpacePos.w;

    // Convert NDC to screen-space UVs (0 to 1)
    glm::vec2 screenUV = (glm::vec2(ndcPos) + 1.0f) * 0.5f;

    // Calculate the background's size in screen space
    // Assuming the background is a quad, we only care about the X and
    // Y scale
    glm::vec2 backgroundSize =
        glm::vec2(backgroundScale.x, backgroundScale.y) /
        viewportSize;

    // Calculate min and max UVs
    minUV = screenUV - backgroundSize * 0.5f;
    maxUV = screenUV + backgroundSize * 0.5f;
}

void BackgroundSpriteStartSys()
{
    backgroundShader = Shader("shaders/background_vertex.shad",
                              "shaders/background_fragment.shad");

    glGenFramebuffers(1, &backgroundFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);

    glGenTextures(1, &renderPassTexture);
    glBindTexture(GL_TEXTURE_2D, renderPassTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, engineState.window->width,
                 engineState.window->height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                    GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, renderPassTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (EntityID ent :
         SceneView<BackgroundSprite>(engineState.scene))
    {
        auto bs = engineState.scene.Get<BackgroundSprite>(ent);

        backgroundShader.use();
        backgroundShader.setVec2(
            "spritePassSize", glm::vec2(engineState.window->width,
                                        engineState.window->height));
        backgroundShader.setVec2(
            "texture1Size",
            glm::vec2(bs->dimensions.x, bs->dimensions.y));
        backgroundShader.setMat4("projection", engineState.projMat);
        backgroundShader.setMat4("spriteProjectionMatrix",
                                 engineState.projMat);
    }
}

struct Rect
{
    glm::vec2 center = {}, halfWidths = {};
};

bool IsCollisionPixel(unsigned char* data, int width, int x, int y)
{
    int idx = (y * width + x) *
              4;           // Assuming RGBA format (4 bytes per pixel)
    return data[idx] < 80; // Check if red channel < 80
}

// Helper to check if an entire tile is a collider
bool IsTileCollider(unsigned char* data, int width, int tileX,
                    int tileY, int tileSize)
{
    // Check at least one pixel in the tile
    for (int y = 0; y < tileSize; y++)
    {
        for (int x = 0; x < tileSize; x++)
        {
            int pixelX = tileX * tileSize + x;
            int pixelY = tileY * tileSize + y;
            if (IsCollisionPixel(data, width, pixelX, pixelY))
            {
                return true;
            }
        }
    }
    return false;
}

// Convert grid coordinates to a unique key for the set
size_t CoordToKey(int x, int y)
{
    return (static_cast<size_t>(y) << 32) | static_cast<size_t>(x);
}

// Merges adjacent tiles into larger rectangles
std::vector<Rect>
MergeTiles(const std::unordered_set<size_t>& collisionTiles,
           int gridWidth, int gridHeight, float tileWorldSize,
           glm::vec2 worldPos)
{
    // Create a 2D grid representation
    std::vector<std::vector<bool>> grid(
        gridHeight, std::vector<bool>(gridWidth, false));

    // Fill the grid
    for (size_t key : collisionTiles)
    {
        int x = key & 0xFFFFFFFF;
        int y = key >> 32;
        if (x < gridWidth && y < gridHeight)
        {
            grid[y][x] = true;
        }
    }

    std::vector<Rect>              mergedRects;
    std::vector<std::vector<bool>> visited(
        gridHeight, std::vector<bool>(gridWidth, false));

    // Try to merge tiles horizontally and vertically
    for (int y = 0; y < gridHeight; y++)
    {
        for (int x = 0; x < gridWidth; x++)
        {
            if (!grid[y][x] || visited[y][x])
                continue;

            // Start with a single tile
            int width = 1;
            int height = 1;

            // Extend horizontally as far as possible
            while (x + width < gridWidth && grid[y][x + width] &&
                   !visited[y][x + width])
            {
                width++;
            }

            // Try to extend vertically
            bool canExtendVertically = true;
            while (canExtendVertically && y + height < gridHeight)
            {
                // Check if we can add an entire row
                for (int i = 0; i < width; i++)
                {
                    if (!grid[y + height][x + i] ||
                        visited[y + height][x + i])
                    {
                        canExtendVertically = false;
                        break;
                    }
                }

                if (canExtendVertically)
                {
                    height++;
                }
            }

            // Mark all cells in this rectangle as visited
            for (int j = 0; j < height; j++)
            {
                for (int i = 0; i < width; i++)
                {
                    visited[y + j][x + i] = true;
                }
            }

            // Calculate world position for this rectangle
            float worldX =
                worldPos.x + (x + width / 2.0f) * tileWorldSize;
            float worldY =
                worldPos.y + (y + height / 2.0f) * tileWorldSize;

            // Add the merged rectangle
            Rect rect;
            rect.center = glm::vec2(worldX, worldY);
            rect.halfWidths =
                glm::vec2(width * tileWorldSize / 2.0f,
                          height * tileWorldSize / 2.0f);
            mergedRects.push_back(rect);
        }
    }

    return mergedRects;
}

void BackgroundSpriteCollidersStartSys()
{
    for (EntityID ent :
         SceneView<BackgroundSprite>(engineState.scene))
    {
        auto bs = engineState.scene.Get<BackgroundSprite>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        const int TILE_SIZE = 16; // 16x16 tiles

        int width = bs->dimensions.x;
        int height = bs->dimensions.y;

        // Calculate number of tiles in each dimension
        int tilesX = width / TILE_SIZE;
        int tilesY = height / TILE_SIZE;

        // Calculate world-space tile size
        float tileWorldSizeX = trans->scale.x / tilesX;
        float tileWorldSizeY = trans->scale.y / tilesY;
        float tileWorldSize = tileWorldSizeX; // Assuming square tiles
                                              // in world space

        // Set to store coordinates of collision tiles
        std::unordered_set<size_t> collisionTiles;

        // Scan all tiles
        for (int tileY = 0; tileY < tilesY; tileY++)
        {
            for (int tileX = 0; tileX < tilesX; tileX++)
            {
                if (IsTileCollider(bs->data, width, tileX, tileY,
                                   TILE_SIZE))
                {
                    collisionTiles.insert(CoordToKey(tileX, tileY));
                }
            }
        }

        // auto tiles = MergeTiles(collisionTiles, tilesX, tilesY,
        //                         tileWorldSize, trans->position);

        // for (Rect& rect : tiles)
        // {
        //     EntityID colEnt = engineState.scene.AddEntity();
        //     engineState.scene.AssignParam<Name>(colEnt,
        //                                         "TileCollider");
        //     auto colTrans = engineState.scene.AssignParam<Transform>(
        //         colEnt, glm::vec3(rect.center, 0.0f), glm::vec3(0.0f),
        //         glm::vec3(rect.halfWidths.x, rect.halfWidths.y, 0.0f));
        //     auto colRigid =
        //         engineState.scene.AssignParam<sm2d::Rigidbody>(
        //             colEnt, sm2d::BodyType::sm2d_Static, colTrans,
        //             1.0f, false, 0.98f, 0.98f, 0.1f, true, 0.5f, 0,
        //             false, false);
        //     engineState.scene.AssignParam<sm2d::Collider>(
        //         colEnt, sm2d::ColliderType::sm2d_AABB,
        //         sm2d::ColAABB(rect.halfWidths), colRigid, false);
        // }
    }
}

REGISTER_START_SYSTEM(BackgroundSpriteCollidersStartSys);
REGISTER_EDITOR_START_SYSTEM(BackgroundSpriteStartSys);

void RenderQuadSys()
{
    Renderer::RenderQuad(
        glm::vec2(engineState.window->width * 0.5f,   // Center X
                  engineState.window->height * 0.5f), // Center Y
        glm::vec2(engineState.window->width,          // Full width
                  engineState.window->height),        // Full height
        0.0f,                                         // No rotation
        engineState.orthoProjMat, renderPassTexture, glm::vec4(1.0f));
}

void BackgroundSpriteSys()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (EntityID ent :
         SceneView<BackgroundSprite>(engineState.scene))
    {
        auto bs = engineState.scene.Get<BackgroundSprite>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        backgroundShader.use();
        backgroundShader.setTexture2D("texture1", bs->texture, 0);
        backgroundShader.setTexture2D("spritePass", renderPassTexture,
                                      1);
        backgroundShader.setTexture2D("depthTexture",
                                      bs->depthTexture, 2);
        backgroundShader.setTexture2D("paletteTexture",
                                      bs->paletteTexture, 3);

        glm::mat4 transform = glm::mat4(1.0f);

        // Matrix multiplication to calculate the transform.
        transform = glm::translate(
            transform, glm::vec3(trans->position.x, trans->position.y,
                                 trans->position.z));

        transform = glm::rotate(transform, trans->rotation.z,
                                glm::vec3(0.0f, 0.0f, 1.0f));

        transform =
            glm::scale(transform, glm::vec3(trans->scale.x,
                                            trans->scale.y, 1.0f));

        // Setting all the uniforms.
        backgroundShader.setMat4("model", transform);
        backgroundShader.setMat4("view",
                                 engineState.camera->GetViewMatrix());

        glBindVertexArray(Renderer::VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

void BackgroundSpriteDraw(BackgroundSprite* sprite)
{
    if (ImGui::CollapsingHeader("BackgroundSprite"))
    {
        char texBuffer[250];
        strncpy_s(texBuffer, sprite->texturePath.c_str(),
                  sizeof(texBuffer));
        if (ImGui::InputText("BsTexturePath", texBuffer,
                             sizeof(texBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            glm::vec2 dim;
            sprite->texturePath = std::string(texBuffer);
            glGenTextures(1, &sprite->texture);

            int width, height, nrComponents;
            sprite->data = stbi_load(texBuffer, &width, &height,
                                     &nrComponents, 0);
            if (sprite->data)
            {
                GLenum format;
                if (nrComponents == 1)
                    format = GL_RED;
                else if (nrComponents == 3)
                    format = GL_RGB;
                else if (nrComponents == 4)
                    format = GL_RGBA;

                glBindTexture(GL_TEXTURE_2D, sprite->texture);
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                             0, format, GL_UNSIGNED_BYTE,
                             sprite->data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                                format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                                  : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                                format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                                  : GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                GL_NEAREST);

                sprite->dimensions = glm::vec2(width, height);

                glBindTexture(GL_TEXTURE_2D, 0);
            }
            else
            {
                std::cout << "Texture failed to load at path: "
                          << sprite->texturePath << '\n';
            }
        }

        char depthTexBuffer[250];
        strncpy_s(depthTexBuffer, sprite->depthTexturePath.c_str(),
                  sizeof(depthTexBuffer));
        if (ImGui::InputText("BsDepthTexturePath", depthTexBuffer,
                             sizeof(depthTexBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            sprite->depthTexturePath = std::string(depthTexBuffer);
            sprite->depthTexture = Utils::LoadTexture(depthTexBuffer);
        }

        char palTexBuffer[250];
        strncpy_s(palTexBuffer, sprite->paletteTexturePath.c_str(),
                  sizeof(palTexBuffer));
        if (ImGui::InputText("BsPalTexturePath", palTexBuffer,
                             sizeof(palTexBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            sprite->paletteTexturePath = std::string(palTexBuffer);
            sprite->paletteTexture = Utils::LoadTexture(palTexBuffer);
        }
    }
}

nlohmann::json BackgroundSpriteSave(BackgroundSprite* sprite)
{
    return {{"BsTexturePath", sprite->texturePath},
            {"BsDepthTexturePath", sprite->depthTexturePath},
            {"BsPalletteTexturePath", sprite->paletteTexturePath},
            {"BsDimensions",
             {sprite->dimensions.x, sprite->dimensions.y}}};
}

void BackgroundSpriteLoad(BackgroundSprite*     sprite,
                          const nlohmann::json& j)
{
    sprite->texturePath = j["BsTexturePath"];
    sprite->texture = Utils::LoadTexture(sprite->texturePath.c_str());
    sprite->depthTexturePath = j["BsDepthTexturePath"];
    sprite->depthTexture =
        Utils::LoadTexture(sprite->depthTexturePath.c_str());
    if (j.contains("BsPalletteTexturePath"))
    {
        sprite->paletteTexturePath = j["BsPalletteTexturePath"];
        glGenTextures(1, &sprite->paletteTexture);

        int width, height, nrComponents;
        sprite->data = stbi_load(sprite->paletteTexturePath.c_str(),
                                 &width, &height, &nrComponents, 0);
        if (sprite->data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            std::cout << "nrComponents: " << nrComponents << '\n';

            glBindTexture(GL_TEXTURE_2D, sprite->paletteTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                         format, GL_UNSIGNED_BYTE, sprite->data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                              : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                            format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                              : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                            GL_NEAREST);

            sprite->dimensions = glm::vec2(width, height);

            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            std::cout << "Texture failed to load at path: "
                      << sprite->paletteTexturePath << '\n';
        }
    }
    if (j.contains("BsDimensions"))
        sprite->dimensions = {j["BsDimensions"][0],
                              j["BsDimensions"][1]};
}

REGISTER_COMPONENT(BackgroundSprite, BackgroundSpriteDraw,
                   BackgroundSpriteSave, BackgroundSpriteLoad);
