#include <syslocked/components.h>
#include <ecs.h>
#include <engine.h>
#include <input.h>
#include <physics.h>
#include <components.h>
#include <utils.h>

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


// Damping factor to slow the player down when no input is applied (simulates friction)
float damping = 0.9f;  // You can tweak this value for more or less friction
float maxSpeed = 10000.0f; // Maximum speed
float forceStrength = 1100000.0f; // Adjust this for how quickly the player accelerates

void PlayerMovementSys()
{
    for (EntityID ent : SceneView<PlayerMovement>(engineState.scene))
    {
        auto player = engineState.scene.Get<PlayerMovement>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);
        Camera* camera = engineState.camera;
        auto playerID = engineState.scene.Get<RigidBody3D>(ent)->body->GetID();
        auto playerBody = engineState.scene.Get<RigidBody3D>(ent)->body;
        auto playerRigid = engineState.scene.Get<RigidBody3D>(ent);

        float speed = player->speed;

        bodyInterface.SetRotation(playerID, JPH::Quat::sIdentity(), JPH::EActivation::Activate);
    
        JPH::Vec3 playerPosition = bodyInterface.GetPosition(playerID);
        JPH::Vec3 playerVelocity = bodyInterface.GetLinearVelocity(playerID);

        JPH::Vec3 force(0.0f, 0.0f, 0.0f); // Initialize a force vector

        // Zero out the Y component of the camera's front direction before normalization
        glm::vec3 frontNoY = glm::vec3(camera->Front.x, 0.0f, camera->Front.z);
        glm::vec3 normalizedFront = glm::normalize(frontNoY);  // Now normalize the horizontal direction

        if (Input::GetKey(Key::W))
        {
            force += JPH::Vec3(normalizedFront.x, 0.0f, normalizedFront.z) * forceStrength;
        }
        if (Input::GetKey(Key::S))
        {
            force -= JPH::Vec3(normalizedFront.x, 0.0f, normalizedFront.z) * forceStrength;
        }
        if (Input::GetKey(Key::A))
        {
            force -= JPH::Vec3(camera->Right.x, 0.0f, camera->Right.z) * forceStrength;
        }
        if (Input::GetKey(Key::D))
        {
            force += JPH::Vec3(camera->Right.x, 0.0f, camera->Right.z) * forceStrength;
        }

        // Dampen the velocity to avoid sliding when no force is applied
        JPH::Vec3 currentVelocity = bodyInterface.GetLinearVelocity(playerID);
        JPH::Vec3 horizontalVelocity = JPH::Vec3(currentVelocity.GetX(), 0.0f, currentVelocity.GetZ()) * damping; // Dampen X and Z
        JPH::Vec3 newVelocity = JPH::Vec3(horizontalVelocity.GetX(), currentVelocity.GetY(), horizontalVelocity.GetZ()); // Keep Y unchanged

        // Clamp velocity to avoid exceeding maximum speed
        if (glm::length(glm::vec2(newVelocity.GetX(), newVelocity.GetZ())) > maxSpeed)
        {
            glm::vec2 clampedVelocity = glm::normalize(glm::vec2(newVelocity.GetX(), newVelocity.GetZ())) * maxSpeed;
            newVelocity.SetX(clampedVelocity.x);
            newVelocity.SetZ(clampedVelocity.y);
        }

        float rayOffset = -2.3f;
        float rayLength = 0.1f;

        JPH::Vec3 rayStart = playerPosition + JPH::Vec3(0, -playerRigid->capsuleHeight / 2 + rayOffset, 0);
        JPH::Vec3 rayEnd = rayStart + JPH::Vec3(0, -rayLength, 0);
        JPH::RRayCast ray(rayStart, rayEnd - rayStart);

        // Create a result object to store the raycast hit data
        JPH::RayCastResult rayCastResult;

        const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

        bool hit = query.CastRay(ray, rayCastResult);

        // Set the new position and velocity for the player
        bodyInterface.SetPosition(playerID, playerPosition, JPH::EActivation::Activate());
        
        bodyInterface.SetLinearVelocity(playerID, newVelocity);
        bodyInterface.AddForce(playerID, force);

        bodyInterface.ActivateBody(playerID); 

        if (Input::GetKeyDown(Key::Space) && hit)
        {
            // Adjust the force value based on your object's mass
            JPH::Vec3 jumpForce(0.0f, 10000000.0f, 0.0f); 
            bodyInterface.AddForce(playerID, jumpForce * player->jumpSpeed);
        }
    }
}

void GunStartSys()
{
    for (EntityID ent : SceneView<Gun>(engineState.scene))
    {
        auto gun = engineState.scene.Get<Gun>(ent);
        gun->soundDevice = SoundDevice::Get();
        gun->shootSound = SoundBuffer::Get()->AddSoundEffect("res/sounds/Shoot.wav");

        gun->soundSource = std::make_shared<SoundSource>();
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

            // Play shoot sound
            gun->soundSource->Play(gun->shootSound);

            // Shoot gun
            for (int i = 0; i < 6; ++i)
            {
                EntityID bulletEnt = engineState.scene.AddEntity();
                engineState.scene.AssignParam<Transform>(bulletEnt, objectTransform->position, glm::vec3(0.0f), glm::vec3(0.065f));
                engineState.scene.AssignParam<MeshRenderer>(bulletEnt, gun->bulletModel, glm::vec4(1.0f), gun->bulletTexture);
                engineState.scene.AssignParam<RigidBody3D>(bulletEnt, ColliderType::Capsule, BodyState::Dynamic, 0.2f, 0.1f, RigidbodyID_Bullet);
                engineState.scene.Assign<Bullet>(bulletEnt);
 
                RigidBody3DStartSys();

                glm::vec3 randomizedBulletDir = camera->Front;
                randomizedBulletDir *= Utils::GenerateRandomNumber(1.0f, 4.0f);
                JPH::Vec3 bulletDirection(randomizedBulletDir.x, randomizedBulletDir.y, randomizedBulletDir.z);
                bodyInterface.AddForce(engineState.scene.Get<RigidBody3D>(bulletEnt)->body->GetID(), bulletDirection * gun->bulletSpeed);
            }
        } 
    }
}

void DestroyBody(JPH::BodyID id)
{
    bodyInterface.RemoveBody(id);
    bodyInterface.DestroyBody(id);
}

void EnemyStartSys()
{
    for (EntityID ent : SceneView<Enemy>(engineState.scene))
    {
        auto enemy = engineState.scene.Get<Enemy>(ent);
        auto rigid = engineState.scene.Get<RigidBody3D>(ent);
 
        enemy->soundDevice = SoundDevice::Get();
        enemy->hurtSound = SoundBuffer::Get()->AddSoundEffect("res/sounds/EnemyHurt.wav");

        enemy->soundSource = std::make_shared<SoundSource>();

        RigidBody3DStartSys();

        AddCollisionEnterEvent(rigid->body, [ent, enemy, rigid](const JPH::Body* id1, const JPH::Body* id2)      
        {
            // Bullet hit callback!
            
            int group1 = id1->GetCollisionGroup().GetGroupID();
            int group2 = id2->GetCollisionGroup().GetGroupID();

            if (group1 == RigidbodyID_Bullet || group2 == RigidbodyID_Bullet)
            {
                enemy->health--;
                enemy->soundSource->Play(enemy->hurtSound);
            }

            if (enemy->health <= 0)
            {
                if (enemy->isDead) { return; }
                enemy->isDead = true;
            }
        });

        enemy->registeredCollisionIndex = registeredCollisions.size() - 1;
    }
}

void EnemySys()
{
    for (EntityID ent : SceneView<Enemy>(engineState.scene))
    {
        auto enemy = engineState.scene.Get<Enemy>(ent);
        auto rigid = engineState.scene.Get<RigidBody3D>(ent);
        
        if (enemy->isDead)
        {
            engineState.scene.Remove<Enemy>(ent);
            engineState.scene.DestroyEntity(ent);

            DestroyBody(rigid->body->GetID());

            registeredCollisions.erase(registeredCollisions.begin() + enemy->registeredCollisionIndex);

        }
    }
}

// Start systems
REGISTER_START_SYSTEM(EnemyStartSys);
REGISTER_START_SYSTEM(GunStartSys);

// Regular systems
REGISTER_SYSTEM(GunSys);
REGISTER_SYSTEM(EnemySys);
REGISTER_SYSTEM(PlayerMovementSys);
REGISTER_SYSTEM(CameraMoveSys);
REGISTER_SYSTEM(CameraLookSys);
