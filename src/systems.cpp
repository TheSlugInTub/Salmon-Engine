#include <components.h>
#include <ecs.h>
#include <engine.h>
#include <iostream>
#include <renderer.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

void MeshRendererSys()
{
    float aspectRatio = engineState.window->GetAspectRatio();

    //glClear(GL_DEPTH_BUFFER_BIT);

    for (auto& light : Renderer::lights)
    {
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

void RigidBody3DStartSys()
{
    BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();

    for (EntityID ent : SceneView<Transform, RigidBody3D>(engineState.scene)) // Loops over all entities with transform and rigidbody components
    {
	auto rigid = engineState.scene.Get<RigidBody3D>(ent);
	auto trans = engineState.scene.Get<Transform>(ent);

	Vec3 transPosition(trans->position.x, trans->position.y, trans->position.z);
	RVec3 RtransPosition = transPosition;

	if (rigid->colliderType == ColliderType::Box)
	{
	    Vec3 bodyScale(rigid->boxSize.x, rigid->boxSize.y, rigid->boxSize.z);

	    BoxShapeSettings floor_shape_settings(bodyScale);
	    floor_shape_settings.SetEmbedded(); 

	    // Create the shape
	    ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	    ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

            // Step 1: Convert Euler angles to GLM quaternion
            glm::quat glmQuat = glm::quat(trans->rotation);

            // Step 2: Convert GLM quaternion to Jolt quaternion
            Quat joltQuat(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w);  // Jolt uses (x, y, z, w) order

	    // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	    BodyCreationSettings floor_settings(floor_shape, RtransPosition, joltQuat,
                                                rigid->state == BodyState::Dynamic ? EMotionType::Dynamic : EMotionType::Static,
                                                rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

	    // Create the actual rigid body
	    rigid->body = bodyInterface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

		// Add it to the world
	    if (rigid->body != nullptr)
	    {
		bodyInterface.AddBody(rigid->body->GetID(), EActivation::Activate);
	    }
        }
        else if (rigid->colliderType == ColliderType::Capsule)
        {
            float radius = rigid->capsuleRadius; // Define capsule radius in RigidBody3D
            float height = rigid->capsuleHeight; // Define capsule height in RigidBody3D

            // Capsule is defined by its half height (distance between the centers of the hemispheres)
            CapsuleShapeSettings capsule_shape_settings(height * 0.5f, radius);
            capsule_shape_settings.SetEmbedded();

            // Step 1: Convert Euler angles to GLM quaternion
            glm::quat glmQuat = glm::quat(trans->rotation);

            // Step 2: Convert GLM quaternion to Jolt quaternion
            Quat joltQuat(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w);  // Jolt uses (x, y, z, w) order

            // Create the shape
            ShapeSettings::ShapeResult capsule_shape_result = capsule_shape_settings.Create();
            ShapeRefC capsule_shape = capsule_shape_result.Get();

            BodyCreationSettings capsule_settings(capsule_shape, RtransPosition, joltQuat,
                                                  rigid->state == BodyState::Dynamic ? EMotionType::Dynamic : EMotionType::Static,
                                                  rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

            // Create the actual rigid body
            rigid->body = bodyInterface.CreateBody(capsule_settings);

            // Add it to the world
            if (rigid->body != nullptr)
            {
                bodyInterface.AddBody(rigid->body->GetID(), EActivation::Activate);
            }
        }
        else if (rigid->colliderType == ColliderType::Sphere)
        {
            float radius = rigid->sphereRadius; // Define sphere radius in RigidBody3D

            // Sphere is defined by its radius
            SphereShapeSettings sphere_shape_settings(radius);
            sphere_shape_settings.SetEmbedded();

            // Create the shape
            ShapeSettings::ShapeResult sphere_shape_result = sphere_shape_settings.Create();
            ShapeRefC sphere_shape = sphere_shape_result.Get();

            BodyCreationSettings sphere_settings(sphere_shape, RtransPosition, Quat::sIdentity(),
                                          rigid->state == BodyState::Dynamic ? EMotionType::Dynamic : EMotionType::Static,
                                          rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

            // Create the actual rigid body
            rigid->body = bodyInterface.CreateBody(sphere_settings);

            // Add it to the world
            if (rigid->body != nullptr)
            {
                bodyInterface.AddBody(rigid->body->GetID(), EActivation::Activate);
            }
        }
        else if (rigid->colliderType == ColliderType::Mesh)
        {
            Model& model = engineState.scene.Get<MeshRenderer>(ent)->model;

            model.extractCollisionMesh();

            JPH::Array<JPH::Float3> vertices;
            vertices.reserve(model.colliderVertices.size() / 3); // 3 floats per vertex

            for (size_t i = 0; i < model.colliderVertices.size(); i += 3) {
                vertices.push_back(JPH::Float3(
                    model.colliderVertices[i],
                    model.colliderVertices[i + 1],
                    model.colliderVertices[i + 2]
                ));
            }

            JPH::Array<JPH::Triangle> triangles;
            triangles.reserve(model.colliderIndices.size() / 3); // 3 indices per triangle

            //for (size_t i = 0; i < model.colliderIndices.size(); i += 3) {
            //    triangles.push_back(JPH::Triangle(
            //        model.colliderIndices[i],
            //        model.colliderIndices[i + 1],
            //        model.colliderIndices[i + 2]
            //    ));
            //}

            for (size_t i = 0; i < model.colliderIndices.size(); i += 3)
            {
                // Get the indices for this triangle
                uint32_t idx1 = model.colliderIndices[i];
                uint32_t idx2 = model.colliderIndices[i + 1];
                uint32_t idx3 = model.colliderIndices[i + 2];

                // Get the corresponding vertices
                glm::vec3 v1(model.colliderVertices[idx1 * 3 + 0], model.colliderVertices[idx1 * 3 + 1], model.colliderVertices[idx1 * 3 + 2]);
                glm::vec3 v2(model.colliderVertices[idx2 * 3 + 0], model.colliderVertices[idx2 * 3 + 1], model.colliderVertices[idx2 * 3 + 2]);
                glm::vec3 v3(model.colliderVertices[idx3 * 3 + 0], model.colliderVertices[idx3 * 3 + 1], model.colliderVertices[idx3 * 3 + 2]);

                // Convert glm::vec3 to Jolt's Vec3 format
                Vec3 joltV1(v1.x, v1.y, v1.z);
                Vec3 joltV2(v2.x, v2.y, v2.z);
                Vec3 joltV3(v3.x, v3.y, v3.z);
    
                // Create a triangle and add it to the list
                triangles.push_back(Triangle(joltV1, joltV2, joltV3));
            }

            // Step 3: Create a material list (with a default material)
            JPH::PhysicsMaterialList material_list;
            material_list.push_back(new JPH::PhysicsMaterial());

            // Step 4: Create the MeshShapeSettings
            JPH::MeshShapeSettings mesh_shape_settings(triangles, material_list);
            mesh_shape_settings.SetEmbedded();  // This embeds the shape into the body

            // Step 5: Convert the object's rotation to Jolt quaternion
            glm::quat glmQuat = glm::quat(trans->rotation); // Step 1: Convert Euler angles to GLM quaternion
            Quat joltQuat(glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w);  // Jolt uses (x, y, z, w) order

            // Step 6: Create the shape
            JPH::ShapeSettings::ShapeResult mesh_shape_result = mesh_shape_settings.Create();
            JPH::ShapeRefC mesh_shape = mesh_shape_result.Get();

            // Step 7: Create body settings
            JPH::BodyCreationSettings mesh_settings(mesh_shape, RtransPosition, joltQuat,
                rigid->state == BodyState::Dynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                rigid->state == BodyState::Dynamic ? Layers::MOVING : Layers::NON_MOVING);

            // Step 8: Create the actual rigid body and add it to the world
            rigid->body = bodyInterface.CreateBody(mesh_settings);
            if (rigid->body != nullptr) {
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
    BodyInterface &bodyInterface = physicsSystem.GetBodyInterface();
    float cDeltaTime = 1.0f / 120.0f;

    for (EntityID ent : SceneView<Transform, RigidBody3D>(engineState.scene))
    {
	auto rigid = engineState.scene.Get<RigidBody3D>(ent);

	RVec3 positionOfSphere = bodyInterface.GetCenterOfMassPosition(rigid->body->GetID());

	Quat rotationOfSphere = bodyInterface.GetRotation(rigid->body->GetID());

	float x = rotationOfSphere.GetX();
	float y = rotationOfSphere.GetY();
	float z = rotationOfSphere.GetZ();
	float w = rotationOfSphere.GetW();
	// Sync box position and rotation with Jolt Physics
	engineState.scene.Get<Transform>(ent)->position = glm::vec3(positionOfSphere.GetX(), positionOfSphere.GetY(), positionOfSphere.GetZ());

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

        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 1.0f, 25.0f);

        light->shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        light->shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        light->shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
        light->shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
        light->shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
        light->shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

        glGenFramebuffers(1, &light->depthMapFBO);
    
        // Create depth cubemap texture
        glGenTextures(1, &light->depthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, light->depthCubemap);
    
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
    
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
        // Attach depth cubemap as the FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, light->depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light->depthCubemap, 0);
        glDrawBuffer(GL_NONE);  // No color buffer is drawn
        glReadBuffer(GL_NONE);  // No need to read from a buffer
    
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
        animator->boneMatrices.reserve(100);

	for (int i = 0; i < 100; i++)
	    animator->boneMatrices.push_back(glm::mat4(1.0f));
    }
}

void AnimatorSys()
{
    for (EntityID ent : SceneView<Animator>(engineState.scene))
    {
        auto anim = engineState.scene.Get<Animator>(ent);

        if (anim->playing)
        {
            UpdateAnimation(0.016f, anim);
        }
    }
}

// Start systems
REGISTER_START_SYSTEM(RigidBody3DStartSys);
REGISTER_START_SYSTEM(LightStartSys);
REGISTER_START_SYSTEM(AnimatorStartSys);

// Regular systems
REGISTER_SYSTEM(AnimatorSys);
REGISTER_SYSTEM(RigidBody3DSys);
REGISTER_SYSTEM(MeshRendererSys);
