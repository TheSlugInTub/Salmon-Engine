#include "salmon/ecs.h"
#include <salmon/renderer.h>
#include <bombratter/background_sprite.h>
#include <imgui/imgui.h>
#include <salmon/json.hpp>
#include <salmon/editor.h>

void BackgroundSpriteStartSys()
{
    backgroundShader = Shader("shaders/background_vertex.shad",
                              "shaders/background_fragment.shad");
}

REGISTER_EDITOR_START_SYSTEM(BackgroundSpriteStartSys);

void BackgroundSpriteSys()
{
    for (EntityID ent :
         SceneView<BackgroundSprite>(engineState.scene))
    {
        auto bs = engineState.scene.Get<BackgroundSprite>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        backgroundShader.use();
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
        backgroundShader.setMat4("projection", engineState.projMat);

        glBindVertexArray(Renderer::VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

void GrabRenderPass(int width, int height)
{
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &backgroundFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);

    // Create the texture to store the sprites
    glGenTextures(1, &renderPassTexture);
    glBindTexture(GL_TEXTURE_2D, renderPassTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, renderPassTexture, 0);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer is not complete!" << std::endl;
    }

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GrabRenderPassSys()
{
    GrabRenderPass(engineState.window->width, engineState.window->height);
}

REGISTER_EDITOR_SYSTEM(GrabRenderPassSys);

void BackgroundSpriteDraw(BackgroundSprite* sprite)
{
    if (ImGui::CollapsingHeader("BackgroundSprite"))
    {
        char texBuffer[128];
        strncpy_s(texBuffer, sprite->texturePath.c_str(),
                  sizeof(texBuffer));
        if (ImGui::InputText("BsTexturePath", texBuffer,
                             sizeof(texBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue))
        {
            sprite->texturePath = std::string(texBuffer);
            sprite->texture = Utils::LoadTexture(texBuffer);
        }

        char depthTexBuffer[128];
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
    return {{"TexturePath", sprite->texturePath},
            {"DepthTexturePath", sprite->depthTexturePath}};
}

void BackgroundSpriteLoad(BackgroundSprite*     sprite,
                          const nlohmann::json& j)
{
    sprite->texturePath = j["TexturePath"];
    sprite->depthTexturePath = j["DepthTexturePath"];
    sprite->texture = Utils::LoadTexture(sprite->texturePath.c_str());
    sprite->depthTexture =
        Utils::LoadTexture(sprite->depthTexturePath.c_str());
}

REGISTER_COMPONENT(BackgroundSprite, BackgroundSpriteDraw,
                   BackgroundSpriteSave, BackgroundSpriteLoad);
