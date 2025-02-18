#include <salmon/editor.h>
#include <bombratter/background_sprite.h>
#include <salmon/components.h>
#include <glm/gtx/string_cast.hpp>

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

    // for (EntityID ent :
    //      SceneView<BackgroundSprite>(engineState.scene))
    // {
    //     auto bs = engineState.scene.Get<BackgroundSprite>(ent);

    //     backgroundShader.use();
    // }
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
        backgroundShader.setTexture2D("spritePass", renderPassTexture,
                                      1);

        glm::vec2 minUV = glm::vec2(0.0f), maxUV = glm::vec2(0.0f);

        CalculateBackgroundScreenMinMax(
            trans->position, trans->scale,
            engineState.camera->GetViewMatrix(), engineState.projMat,
            glm::vec2(engineState.window->width,
                      engineState.window->height),
            minUV, maxUV);

        backgroundShader.setVec2("backgroundScreenMin", minUV);
        backgroundShader.setVec2("backgroundScreenMax", maxUV);
        backgroundShader.setVec2(
            "viewportSize", glm::vec2(engineState.window->width,
                                      engineState.window->height));
        backgroundShader.setMat4("projection", engineState.projMat);
        backgroundShader.setTexture2D("depthTexture",
                                      bs->depthTexture, 2);
        backgroundShader.setTexture2D("texture1", bs->texture, 0);

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
            sprite->texturePath = std::string(texBuffer);
            sprite->texture = Utils::LoadTexture(texBuffer);
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
    }
}

nlohmann::json BackgroundSpriteSave(BackgroundSprite* sprite)
{
    return {{"BsTexturePath", sprite->texturePath},
            {"BsDepthTexturePath", sprite->depthTexturePath}};
}

void BackgroundSpriteLoad(BackgroundSprite*     sprite,
                          const nlohmann::json& j)
{
    sprite->texturePath = j["BsTexturePath"];
    sprite->texture = Utils::LoadTexture(sprite->texturePath.c_str());
    sprite->depthTexturePath = j["BsDepthTexturePath"];
    sprite->depthTexture =
        Utils::LoadTexture(sprite->depthTexturePath.c_str());
}

REGISTER_COMPONENT(BackgroundSprite, BackgroundSpriteDraw,
                   BackgroundSpriteSave, BackgroundSpriteLoad);
