#include <components.h>
#include <ecs.h>
#include <engine.h>
#include <iostream>
#include <renderer.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

void MeshRendererSys()
{
	for (EntityID ent : SceneView<Transform, MeshRenderer>(engineState.scene))
	{
		float aspectRatio = engineState.window->GetAspectRatio();
		Renderer::RenderModel(ent, engineState.camera->GetProjMatrix(aspectRatio), engineState.camera->GetViewMatrix());
	}
}

void CameraMoveSys()
{
	float deltaTime = 0.016f;
	if (glfwGetKey(engineState.window->window, GLFW_KEY_W) == GLFW_PRESS)
		engineState.camera->ProcessKeyboard(CameraMovement::FORWARD, deltaTime);
	if (glfwGetKey(engineState.window->window, GLFW_KEY_S) == GLFW_PRESS)
		engineState.camera->ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);
	if (glfwGetKey(engineState.window->window, GLFW_KEY_A) == GLFW_PRESS)
		engineState.camera->ProcessKeyboard(CameraMovement::LEFT, deltaTime);
	if (glfwGetKey(engineState.window->window, GLFW_KEY_D) == GLFW_PRESS)
		engineState.camera->ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
}

void CameraLookSys()
{
    // Variables to store the last mouse position
    static bool firstMouse = true;
    static float lastX = 0.0f, lastY = 0.0f;

    // Get the current mouse position
    double xpos, ypos;
    glfwGetCursorPos(engineState.window->window, &xpos, &ypos);

    if (firstMouse)
    {
	lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
	firstMouse = false;
    }

    // Calculate the offset from the last position
    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top

    // Update the last position
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    // Process the mouse movement with the calculated offset
    engineState.camera->ProcessMouseMovement(xoffset, yoffset);
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

	    // Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	    BodyCreationSettings floor_settings(floor_shape, RtransPosition, Quat::sIdentity(),
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

            // Create the shape
            ShapeSettings::ShapeResult capsule_shape_result = capsule_shape_settings.Create();
            ShapeRefC capsule_shape = capsule_shape_result.Get();

            BodyCreationSettings capsule_settings(capsule_shape, RtransPosition, Quat::sIdentity(),
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

REGISTER_START_SYSTEM(RigidBody3DStartSys);
REGISTER_SYSTEM(RigidBody3DSys);
REGISTER_SYSTEM(CameraLookSys);
REGISTER_SYSTEM(MeshRendererSys);
REGISTER_SYSTEM(CameraMoveSys);
