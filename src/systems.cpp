#include <salmon/components.h>
#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>
#include <iostream>
#include <salmon/renderer.h>
#include <GLFW/glfw3.h>
#include <salmon/particle_system.h>
#include <salmon/ui.h>

void MeshRendererSys()
{
    float aspectRatio = engineState.window->GetAspectRatio();

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
            Renderer::depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", light.shadowTransforms[i]);
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
        Renderer::RenderModel(ent, engineState.camera->GetProjMatrix(aspectRatio), engineState.camera->GetViewMatrix());
    }
}

void SpriteRendererSys()
{
    float aspectRatio = engineState.window->GetAspectRatio();

    for (EntityID ent : SceneView<SpriteRenderer>(engineState.scene))
    {
        Renderer::RenderSprite(ent, engineState.camera->GetProjMatrix(aspectRatio),
                               engineState.camera->GetViewMatrix());
    }
}

void RigidBody3DStartSys()
{
    for (EntityID ent : SceneView<Transform, RigidBody3D>(
             engineState.scene)) // Loops over all entities with transform and rigidbody components
    {
        auto rigid = engineState.scene.Get<RigidBody3D>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        if (rigid->body != nullptr)
        {
            continue;
        }

        JPH::Vec3 transPosition(trans->position.x, trans->position.y, trans->position.z);
        JPH::RVec3 RtransPosition = transPosition;

        if (rigid->colliderType == ColliderType::Box)
        {
            JPH::Vec3 bodyScale(rigid->boxSize.x, rigid->boxSize.y, rigid->boxSize.z);

            JPH::BoxShapeSettings floor_shape_settings(bodyScale);
            floor_shape_settings.SetEmbedded();

            // Create the shape
            JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
            JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check
                                                                   // floor_shape_result for HasError() / GetError()

            // Step 1: Convert Euler angles to GLM quaternion
            glm::quat glmQuat = glm::quat(trans->rotation);

            // Step 2: Convert GLM quaternion to Jolt quaternion
            JPH::Quat joltQuat(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w); // Jolt uses (x, y, z, w) order

            // Create the settings for the body itself. Note that here you can also set other properties like the
            // restitution / friction.
            JPH::BodyCreationSettings floor_settings(
                floor_shape, RtransPosition, joltQuat,
                rigid->state == BodyState::Dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

            floor_settings.mCollisionGroup.SetGroupID(rigid->groupID);
            // Create the actual rigid body
            rigid->body =
                bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

            // Add it to the world
            if (rigid->body != nullptr)
            {
                bodyInterface.AddBody(rigid->body->GetID(), JPH::EActivation::Activate);
            }
        }
        else if (rigid->colliderType == ColliderType::Capsule)
        {
            float radius = rigid->capsuleRadius;
            float height = rigid->capsuleHeight;

            // Capsule is defined by its half height (distance between the centers of the hemispheres)
            JPH::CapsuleShapeSettings capsule_shape_settings(height * 0.5f, radius);
            capsule_shape_settings.SetEmbedded();

            // Step 1: Convert Euler angles to GLM quaternion
            glm::quat glmQuat = glm::quat(trans->rotation);

            // Step 2: Convert GLM quaternion to Jolt quaternion
            JPH::Quat joltQuat(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w); // Jolt uses (x, y, z, w) order

            // Create the shape
            JPH::ShapeSettings::ShapeResult capsule_shape_result = capsule_shape_settings.Create();
            JPH::ShapeRefC capsule_shape = capsule_shape_result.Get();

            JPH::BodyCreationSettings capsule_settings(
                capsule_shape, RtransPosition, joltQuat,
                rigid->state == BodyState::Dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

            capsule_settings.mCollisionGroup.SetGroupID(rigid->groupID);
            // Create the actual rigid body
            rigid->body = bodyInterface.CreateBody(capsule_settings);

            // Add it to the world
            if (rigid->body != nullptr)
            {
                bodyInterface.AddBody(rigid->body->GetID(), JPH::EActivation::Activate);
            }
        }
        else if (rigid->colliderType == ColliderType::Sphere)
        {
            float radius = rigid->sphereRadius; // Define sphere radius in RigidBody3D

            // Sphere is defined by its radius
            JPH::SphereShapeSettings sphere_shape_settings(radius);
            sphere_shape_settings.SetEmbedded();

            // Create the shape
            JPH::ShapeSettings::ShapeResult sphere_shape_result = sphere_shape_settings.Create();
            JPH::ShapeRefC sphere_shape = sphere_shape_result.Get();

            JPH::BodyCreationSettings sphere_settings(
                sphere_shape, RtransPosition, JPH::Quat::sIdentity(),
                rigid->state == BodyState::Dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

            sphere_settings.mCollisionGroup.SetGroupID(rigid->groupID);
            // Create the actual rigid body
            rigid->body = bodyInterface.CreateBody(sphere_settings);

            // Add it to the world
            if (rigid->body != nullptr)
            {
                bodyInterface.AddBody(rigid->body->GetID(), JPH::EActivation::Activate);
            }
        }
        else
        {
            std::cerr << "ERROR: Collider type not implemented or is null" << std::endl;
        }
    }
}

void RigidBody3DSys()
{
    for (EntityID ent : SceneView<RigidBody3D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<RigidBody3D>(ent);

        if (rigid->state == BodyState::Static)
        {
            continue;
        }

        JPH::RVec3 positionOfSphere = bodyInterface.GetCenterOfMassPosition(rigid->body->GetID());

        JPH::Quat rotationOfSphere = bodyInterface.GetRotation(rigid->body->GetID());

        float x = rotationOfSphere.GetX();
        float y = rotationOfSphere.GetY();
        float z = rotationOfSphere.GetZ();
        float w = rotationOfSphere.GetW();
        // Sync box position and rotation with Jolt Physics
        engineState.scene.Get<Transform>(ent)->position =
            glm::vec3(positionOfSphere.GetX() + rigid->offset.x, positionOfSphere.GetY() + rigid->offset.y,
                      positionOfSphere.GetZ() + rigid->offset.z);

        glm::quat quatRotation(-z, y, x, w);
        glm::vec3 eulerRotation = glm::eulerAngles(quatRotation); // Converts quaternion to Euler angles

        engineState.scene.Get<Transform>(ent)->rotation = eulerRotation;
    }
}

void LightStartSys()
{
    for (EntityID ent : SceneView<Light>(engineState.scene))
    {
        auto light = engineState.scene.Get<Light>(ent);

        glm::vec3 lightPos = light->position;

        light->shadowTransforms.clear();

        glm::mat4 shadowProj =
            glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 1.0f, 25.0f);

        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        light->shadowTransforms.push_back(
            shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        glGenFramebuffers(1, &light->depthMapFBO);

        // Create depth cubemap texture
        glGenTextures(1, &light->depthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light->depthCubemap);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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

// Start systems
REGISTER_START_SYSTEM(RigidBody3DStartSys);
REGISTER_START_SYSTEM(LightStartSys);
REGISTER_START_SYSTEM(AnimatorStartSys);

// Regular systems
REGISTER_SYSTEM(DeltaTimeSystem);
REGISTER_SYSTEM(AnimatorSys);
REGISTER_SYSTEM(MeshRendererSys);
REGISTER_SYSTEM(SpriteRendererSys);
REGISTER_SYSTEM(RigidBody3DSys);
REGISTER_SYSTEM(ParticleSystemSys);
REGISTER_SYSTEM(ButtonSys);
REGISTER_SYSTEM(TextSys);
