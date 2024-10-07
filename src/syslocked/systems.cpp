#include <syslocked/components.h>
#include <ecs.h>
#include <engine.h>
#include <input.h>
#include <physics.h>
#include <components.h>

void CameraMoveSys()
{
    if (glfwGetKey(engineState.window->window, GLFW_KEY_W) == GLFW_PRESS)
	engineState.camera->ProcessKeyboard(CameraMovement::FORWARD, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_S) == GLFW_PRESS)
	    engineState.camera->ProcessKeyboard(CameraMovement::BACKWARD, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_A) == GLFW_PRESS)
	    engineState.camera->ProcessKeyboard(CameraMovement::LEFT, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_D) == GLFW_PRESS)
	    engineState.camera->ProcessKeyboard(CameraMovement::RIGHT, engineState.deltaTime);
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

inline bool canJump = false;
inline bool jumpRegistered = false;

void TurnOffJump()
{
    canJump = false;
}

void TurnOnJump(JPH::BodyID id1, JPH::BodyID id2)
{
    canJump = true;
}

void PlayerMovementSys()
{
    for (EntityID ent : SceneView<PlayerMovement>(engineState.scene))
    {
        auto player = engineState.scene.Get<PlayerMovement>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);
        Camera* camera = engineState.camera;
        auto playerID = engineState.scene.Get<RigidBody3D>(ent)->body->GetID();
        auto playerBody = engineState.scene.Get<RigidBody3D>(ent)->body;

        float speed = player->speed;

        if (!jumpRegistered)
        {
            auto rigidbody = engineState.scene.Get<RigidBody3D>(ent);
            AddCollisionEnterEvent(rigidbody->body->GetID(), TurnOnJump);
            AddCollisionExitEvent(rigidbody->body->GetID(), TurnOffJump);
            jumpRegistered = true;
        }

        bodyInterface.SetRotation(playerID, Quat::sIdentity(), EActivation::Activate);
    
        Vec3 playerPosition = bodyInterface.GetPosition(playerID);

        // Zero out the Y component of the camera's front direction before normalization
        glm::vec3 frontNoY = glm::vec3(camera->Front.x, 0.0f, camera->Front.z);
        glm::vec3 normalizedFront = glm::normalize(frontNoY);  // Now normalize the horizontal direction

        if (Input::GetKey(Key::W))
        {
            playerPosition += Vec3(normalizedFront.x, 0.0f, normalizedFront.z) * speed;
        }
        if (Input::GetKey(Key::S))
        {
            playerPosition -= Vec3(normalizedFront.x, 0.0f, normalizedFront.z) * speed;
        }
        if (Input::GetKey(Key::A))
        {
            playerPosition -= Vec3(camera->Right.x, 0.0f, camera->Right.z) * speed;
        }
        if (Input::GetKey(Key::D))
        {
            playerPosition += Vec3(camera->Right.x, 0.0f, camera->Right.z) * speed;
        }

        // Set the new position for the player
        bodyInterface.SetPosition(playerID, playerPosition, EActivation::Activate());
    
        bodyInterface.ActivateBody(playerID);

        if (Input::GetKeyDown(Key::Space) && canJump)
        {
            // Adjust the force value based on your object's mass
            Vec3 jumpForce(0.0f, 10000000.0f, 0.0f); 
            bodyInterface.AddForce(playerID, jumpForce * player->jumpSpeed);
        }
    }
}

void GunSys()
{
    for (EntityID ent : SceneView<Gun>(engineState.scene))
    {
        Transform* objectTransform = engineState.scene.Get<Transform>(ent);
        auto gun = engineState.scene.Get<Gun>(ent);

        gun->gunTimer -= engineState.deltaTime;

        objectTransform->useMatrix = true;
        objectTransform->modelMat = glm::mat4(1.0f);

        Camera* camera = engineState.camera;
        float forwardOffset = gun->forwardOffset;
        float upOffset = gun->upOffset;
        float rightOffset = gun->rightOffset;
        
        objectTransform->position = camera->Position + camera->Front * forwardOffset;
        objectTransform->position += camera->Right * rightOffset;
        objectTransform->position += camera->Up * upOffset; 

        objectTransform->modelMat = glm::translate(objectTransform->modelMat, objectTransform->position);
        objectTransform->modelMat = glm::scale(objectTransform->modelMat, objectTransform->scale);

        glm::mat4 inverseView = glm::inverse(camera->GetViewMatrix());

        glm::mat4 rotationOnlyInverseView = glm::mat4(1.0f); // Identity matrix
        rotationOnlyInverseView[0] = glm::vec4(inverseView[0]); // Copy the rotation part (X axis)
        rotationOnlyInverseView[1] = glm::vec4(inverseView[1]); // Copy the rotation part (Y axis)
        rotationOnlyInverseView[2] = glm::vec4(inverseView[2]); // Copy the rotation part (Z axis)

        objectTransform->modelMat *= rotationOnlyInverseView;

        auto animator = engineState.scene.Get<Animator>(ent);

        if (Input::GetMouseButtonDown(MouseKey::LeftClick) && gun->gunTimer <= 0)
        {
            animator->currentTime = 0.0f;
            animator->playing = true;
            gun->gunTimer = 0.78f;
        } 
    }
}

// Regular systems
REGISTER_SYSTEM(GunSys);
REGISTER_SYSTEM(PlayerMovementSys);
REGISTER_SYSTEM(CameraMoveSys);
REGISTER_SYSTEM(CameraLookSys);
