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
        auto trans = engineState.scene.Get<Transform>(ent);

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

        glm::vec2 tileSize = bs->dimensions / glm::vec2(trans->scale);
        int       tilesX = static_cast<int>(trans->scale.x);
        int       tilesY = static_cast<int>(trans->scale.y);

        // Create a 2D grid to mark which tiles are colliders
        std::vector<std::vector<bool>> colliderGrid(
            tilesY, std::vector<bool>(tilesX, false));

        // Step 1: Identify tiles with red channel < 80
        for (int tileY = 0; tileY < tilesY; tileY++)
        {
            for (int tileX = 0; tileX < tilesX; tileX++)
            {
                bool isCollider = false;

                // Check all pixels in this tile
                for (int y = 0; y < tileSize.y && !isCollider; y++)
                {
                    for (int x = 0; x < tileSize.x && !isCollider;
                         x++)
                    {
                        // Calculate the pixel position in the image
                        int pixelX = tileX * tileSize.x + x;
                        int pixelY = tileY * tileSize.y + y;

                        // Calculate pixel index in the data array
                        // (assuming RGBA format)
                        int pixelIndex =
                            (pixelY *
                                 static_cast<int>(bs->dimensions.x) +
                             pixelX) *
                            4;

                        // Check if the red channel is less than 80
                        if (bs->data[pixelIndex] <= 80)
                        {
                            isCollider = true;
                        }
                    }
                }

                colliderGrid[tileY][tileX] = isCollider;
            }
        }

        // Step 2: Merge adjacent colliders using a connected
        // component algorithm
        std::vector<std::vector<int>> labelGrid(
            tilesY, std::vector<int>(tilesX, 0));
        int nextLabel = 1;

        // First pass: assign initial labels
        std::map<int, std::vector<glm::ivec2>> labelToTiles;

        for (int y = 0; y < tilesY; y++)
        {
            for (int x = 0; x < tilesX; x++)
            {
                if (colliderGrid[y][x])
                {
                    std::vector<int> neighborLabels;

                    // Check neighbors (4-connected)
                    if (x > 0 && colliderGrid[y][x - 1])
                    {
                        neighborLabels.push_back(labelGrid[y][x - 1]);
                    }
                    if (y > 0 && colliderGrid[y - 1][x])
                    {
                        neighborLabels.push_back(labelGrid[y - 1][x]);
                    }

                    if (neighborLabels.empty())
                    {
                        // New component
                        labelGrid[y][x] = nextLabel++;
                    }
                    else
                    {
                        // Join existing component
                        int smallestLabel =
                            *std::min_element(neighborLabels.begin(),
                                              neighborLabels.end());
                        labelGrid[y][x] = smallestLabel;
                    }

                    // Track tiles for each label
                    labelToTiles[labelGrid[y][x]].push_back(
                        glm::ivec2(x, y));
                }
            }
        }

        // Second pass: resolve label equivalences and ensure adjacent
        // tiles have the same label
        for (int y = 0; y < tilesY; y++)
        {
            for (int x = 0; x < tilesX; x++)
            {
                if (!colliderGrid[y][x])
                    continue;

                // Check right and down neighbors for connected
                // components
                if (x < tilesX - 1 && colliderGrid[y][x + 1] &&
                    labelGrid[y][x] != labelGrid[y][x + 1])
                {
                    int oldLabel = labelGrid[y][x + 1];
                    int newLabel = labelGrid[y][x];

                    // Merge labels
                    for (auto& labeledTile : labelToTiles[oldLabel])
                    {
                        labelGrid[labeledTile.y][labeledTile.x] =
                            newLabel;
                        labelToTiles[newLabel].push_back(labeledTile);
                    }
                    labelToTiles.erase(oldLabel);
                }

                if (y < tilesY - 1 && colliderGrid[y + 1][x] &&
                    labelGrid[y][x] != labelGrid[y + 1][x])
                {
                    int oldLabel = labelGrid[y + 1][x];
                    int newLabel = labelGrid[y][x];

                    // Merge labels
                    for (auto& labeledTile : labelToTiles[oldLabel])
                    {
                        labelGrid[labeledTile.y][labeledTile.x] =
                            newLabel;
                        labelToTiles[newLabel].push_back(labeledTile);
                    }
                    labelToTiles.erase(oldLabel);
                }
            }
        }

        // Step 3: Create box colliders for each connected component
        for (const auto& [label, tiles] : labelToTiles)
        {
            // Find bounds of this connected component
            int minX = tilesX, minY = tilesY, maxX = 0, maxY = 0;

            for (const auto& tile : tiles)
            {
                minX = std::min(minX, tile.x);
                minY = std::min(minY, tile.y);
                maxX = std::max(maxX, tile.x);
                maxY = std::max(maxY, tile.y);
            }

            // Calculate center and half-widths for the box collider
            glm::vec2 center =
                glm::vec2((minX + maxX + 1) * tileSize.x / 2.0f,
                          (minY + maxY + 1) * tileSize.y / 2.0f);

            glm::vec2 halfWidths =
                glm::vec2((maxX - minX + 1) * tileSize.x / 2.0f,
                          (maxY - minY + 1) * tileSize.y / 2.0f);

            EntityID colEnt = engineState.scene.AddEntity();
            auto colTrans = engineState.scene.AssignParam<Transform>(
                colEnt, glm::vec3(center, 0.0f), glm::vec3(0.0f),
                glm::vec3(halfWidths.x, halfWidths.y, 0.0f));
            auto colRigid =
                engineState.scene.AssignParam<sm2d::Rigidbody>(
                    colEnt, sm2d::BodyType::sm2d_Dynamic, colTrans,
                    1.0f, false, 0.98f, 0.98f, 0.1f, true, 0.5f, 0,
                    false, false);
            engineState.scene.AssignParam<sm2d::Collider>(
                colEnt, sm2d::ColliderType::sm2d_AABB,
                sm2d::ColAABB(halfWidths), colRigid, false);
        }
    }
}

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
            sprite->dimensions = dim;
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
