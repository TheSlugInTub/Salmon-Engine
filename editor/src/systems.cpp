#include <salmon/components.h>
#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <salmon/renderer.h>
#include <GLFW/glfw3.h>
#include <salmon/particle_system.h>
#include <salmon/ui.h>
#include <salmon/tilemap.h>

void MeshRendererSys()
{
    for (auto& light : Renderer::lights)
    {
        if (!light.castShadows)
        {
            break;
        }

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, light.depthMapFBO);
        Renderer::depthShader.use();
        glClear(GL_DEPTH_BUFFER_BIT);

        for (unsigned int i = 0; i < 6; ++i)
            Renderer::depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]",
                                          light.shadowTransforms[i]);
        Renderer::depthShader.setFloat("farPlane", light.radius);
        Renderer::depthShader.setVec3("lightPos", light.position);

        for (EntityID ent : SceneView<Transform, MeshRenderer>(engineState.scene))
        {
            auto trans = engineState.scene.Get<Transform>(ent);
            auto model = engineState.scene.Get<MeshRenderer>(ent);

            glm::mat4 transform = Renderer::MakeModelTransform(trans);

            Renderer::depthShader.setMat4("model", transform);
            model->model.Draw(Renderer::depthShader);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the framebuffer after rendering
    }

    glViewport(0, 0, engineState.window->width, engineState.window->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (EntityID ent : SceneView<Transform, MeshRenderer>(engineState.scene))
    {
        Renderer::RenderModel(ent, engineState.projMat, engineState.camera->GetViewMatrix());
    }
}

void SpriteRendererSys()
{
    for (EntityID ent : SceneView<SpriteRenderer>(engineState.scene))
    {
        Renderer::RenderSprite(ent, engineState.projMat, engineState.camera->GetViewMatrix());
    }
}

void LightStartSys()
{
    for (EntityID ent : SceneView<Light>(engineState.scene))
    {
        auto light = engineState.scene.Get<Light>(ent);

        glm::vec3 lightPos = light->position;

        light->shadowTransforms.clear();

        glm::mat4 shadowProj = glm::perspective(
            glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 1.0f, 25.0f);

        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f),
                                     glm::vec3(0.0f, -1.0f, 0.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f),
                                     glm::vec3(0.0f, -1.0f, 0.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f),
                                     glm::vec3(0.0f, 0.0f, 1.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f),
                                     glm::vec3(0.0f, 0.0f, -1.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f),
                                     glm::vec3(0.0f, -1.0f, 0.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f),
                                     glm::vec3(0.0f, -1.0f, 0.0f)));

        glGenFramebuffers(1, &light->depthMapFBO);

        // Create depth cubemap texture
        glGenTextures(1, &light->depthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light->depthCubemap);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
                         SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        // Attach depth cubemap as the FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, light->depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light->depthCubemap, 0);
        glDrawBuffer(GL_NONE); // No color buffer is drawn
        glReadBuffer(GL_NONE); // No need to read from a buffer

        // Ensure framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;

        // Unbind framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Renderer::lights.push_back(*light);
    }
}

void AnimatorStartSys()
{
    for (EntityID ent : SceneView<Animator>(engineState.scene))
    {
        auto animator = engineState.scene.Get<Animator>(ent);

        animator->currentTime = 0.0f;
        animator->boneMatrices.reserve(200);
        animator->model = &engineState.scene.Get<MeshRenderer>(ent)->model;

        for (int i = 0; i < 200; i++) animator->boneMatrices.push_back(glm::mat4(1.0f));
    }
}

void AnimatorSys()
{
    for (EntityID ent : SceneView<Animator>(engineState.scene))
    {
        auto anim = engineState.scene.Get<Animator>(ent);

        if (anim->playing)
        {
            UpdateAnimation(engineState.deltaTime * anim->speed, anim);
        }
    }
}

float lastFrame = 0.0f;

void DeltaTimeSystem()
{
    float currplayerFrame = static_cast<float>(glfwGetTime());
    engineState.deltaTime = currplayerFrame - lastFrame;
    lastFrame = currplayerFrame;
}

// DO NOT LET CLANG FORMAT ANYWHERE NEAR THIS BEAUTY 

// Start systems
REGISTER_START_SYSTEM        (LightStartSys);
REGISTER_EDITOR_START_SYSTEM (AnimatorStartSys);

// Regular systems
REGISTER_EDITOR_SYSTEM (DeltaTimeSystem);
REGISTER_SYSTEM        (AnimatorSys);
REGISTER_EDITOR_SYSTEM (MeshRendererSys);
REGISTER_EDITOR_SYSTEM (SpriteRendererSys);
REGISTER_EDITOR_SYSTEM (ParticleSystemSys);
REGISTER_EDITOR_SYSTEM (ButtonSys);
REGISTER_EDITOR_SYSTEM (TextSys);
REGISTER_EDITOR_SYSTEM (TilemapSys);
