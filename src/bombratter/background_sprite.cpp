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
    int x, y, width, height;
};

void BackgroundSpriteCollidersStartSys()
{
    for (EntityID ent :
         SceneView<BackgroundSprite>(engineState.scene))
    {
        auto bs = engineState.scene.Get<BackgroundSprite>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        const int TILE_SIZE = 16; // 16x16 tiles

        int width = (int)bs->dimensions.x;
        int height = (int)bs->dimensions.y;

        // Calculate number of tiles in each dimension
        int tilesX = width / TILE_SIZE;
        int tilesY = height / TILE_SIZE;

        // Calculate world-space tile size
        float tileWorldSizeX = trans->scale.x / tilesX;
        float tileWorldSizeY = trans->scale.y / tilesY;
        float tileWorldSize = tileWorldSizeX; // Assuming square tiles
                                              // in world space

        std::vector<std::vector<bool>> colliderGrid(
            tilesY, std::vector<bool>(tilesX, false));

        // Analyze image data to determine which tiles need colliders
        for (int y = 0; y < tilesY; y++)
        {
            for (int x = 0; x < tilesX; x++)
            {
                bool createCollider = false;

                // Check each pixel in the tile
                for (int py = 0; py < TILE_SIZE && !createCollider;
                     py++)
                {
                    for (int px = 0;
                         px < TILE_SIZE && !createCollider; px++)
                    {
                        int pixelX = x * TILE_SIZE + px;
                        int pixelY = y * TILE_SIZE + py;

                        if (pixelX < width && pixelY < height)
                        {
                            // Assuming data is stored as RGBA, 4
                            // bytes per pixel
                            int index = (pixelY * width + pixelX) * 4;
                            unsigned char red = bs->data[index];

                            if (red <= 80)
                            {
                                createCollider = true;
                            }
                        }
                    }
                }

                colliderGrid[y][x] = createCollider;
            }
        }

        // Merge adjacent colliders horizontally
        std::vector<Rect> horizontalRects;
        for (int y = 0; y < tilesY; y++)
        {
            int startX = -1;
            for (int x = 0; x <= tilesX; x++)
            {
                if (x < tilesX && colliderGrid[y][x])
                {
                    if (startX == -1)
                    {
                        startX = x;
                    }
                }
                else if (startX != -1)
                {
                    // Found the end of a horizontal strip
                    int endX = x - 1;
                    horizontalRects.push_back(
                        {startX, y, endX - startX + 1, 1});
                    startX = -1;
                }
            }
        }

        // Merge vertical strips
        std::vector<Rect> mergedRects;
        for (size_t i = 0; i < horizontalRects.size(); i++)
        {
            if (horizontalRects[i].width == 0)
                continue; // Skip already merged

            Rect current = horizontalRects[i];

            // Try to extend vertically
            for (size_t j = i + 1; j < horizontalRects.size(); j++)
            {
                Rect& next = horizontalRects[j];

                if (next.width == 0)
                    continue; // Skip already merged

                if (next.x == current.x &&
                    next.width == current.width &&
                    next.y == current.y + current.height)
                {
                    // Can merge vertically
                    current.height += next.height;
                    next.width = 0; // Mark as merged
                }
            }

            mergedRects.push_back(current);
        }

        // Create actual colliders from the merged rectangles
        for (const auto& rect : mergedRects)
        {
            // Calculate world position of the rectangle center
            float worldX =
                trans->position.x - (trans->scale.x / 2.0f) +
                (rect.x + rect.width / 2.0f) * tileWorldSizeX;
            float worldY =
                trans->position.y - (trans->scale.y / 2.0f) +
                (rect.y + rect.height / 2.0f) * tileWorldSizeY;

            // Calculate half-widths
            float halfWidth = (rect.width * tileWorldSizeX) / 2.0f;
            float halfHeight = (rect.height * tileWorldSizeY) / 2.0f;

            // Create the collider
            // MakeCol(glm::vec2(worldX, worldY),
            //         glm::vec2(halfWidth, halfHeight));

            EntityID tileCol = engineState.scene.AddEntity();
            engineState.scene.AssignParam<Name>(tileCol,
                                                "TileCollider");

            auto tileColTrans =
                engineState.scene.AssignParam<Transform>(
                    tileCol, glm::vec3(worldX, worldY, 0.0f),
                    glm::vec3(0.0f), glm::vec3(0.0f));

            auto tileColBody =
                engineState.scene.AssignParam<sm2d::Rigidbody>(
                    tileCol, sm2d::BodyType::sm2d_Static,
                    tileColTrans, 1.0f, false, 0.98f, 0.98f, 0.1f,
                    true, 1.0f, 0, false, false);

            sm2d::ColPolygon poly(
                {glm::vec2(halfWidth, halfHeight),
                 glm::vec2(halfWidth, -halfHeight),
                 glm::vec2(-halfWidth, -halfHeight),
                 glm::vec2(-halfWidth, halfHeight)});

            engineState.scene.AssignParam<sm2d::Collider>(
                tileCol, sm2d::ColliderType::sm2d_Polygon, poly,
                tileColBody);
        }
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
    sprite->depthTexturePath = j["BsDepthTexturePath"];
    sprite->paletteTexturePath = j["BsPalletteTexturePath"];

    sprite->depthTexture =
        Utils::LoadTexture(sprite->depthTexturePath.c_str());
    sprite->paletteTexture =
        Utils::LoadTexture(sprite->paletteTexturePath.c_str());

    glGenTextures(1, &sprite->texture);

    int width, height, nrComponents;
    sprite->data = stbi_load(sprite->texturePath.c_str(), &width,
                             &height, &nrComponents, 0);
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
                  << sprite->texturePath << '\n';
    }
}

REGISTER_COMPONENT(BackgroundSprite, BackgroundSpriteDraw,
                   BackgroundSpriteSave, BackgroundSpriteLoad);
