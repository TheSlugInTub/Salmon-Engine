#include <syslocked/components.h>
#include <ecs.h>
#include <engine.h>
#include <input.h>
#include <physics.h>
#include <components.h>
#include <utils.h>

void StrayStartSys()
{
    for (EntityID ent : SceneView<Stray>(engineState.scene)) { auto rigid = engineState.scene.Get<RigidBody3D>(ent); }
}

void StraySys()
{
    for (EntityID ent : SceneView<Stray>(engineState.scene))
    {
        auto trans = engineState.scene.Get<Transform>(ent);
        auto rigid = engineState.scene.Get<RigidBody3D>(ent);
        auto enemy = engineState.scene.Get<Stray>(ent);
        auto enemyID = rigid->body->GetID();

        JPH::Vec3 currentPosition = rigid->body->GetPosition();
        JPH::Vec3 currentVelocity = rigid->body->GetLinearVelocity();
        glm::vec3 glmTarget = enemy->playerTransform->position;
        JPH::Vec3 target(glmTarget.x, glmTarget.y, glmTarget.z);

        JPH::Vec3 direction = target - currentPosition;
        if (direction.LengthSq() < JPH::Square(0.01f))
        {
            // Target is very close; no need to apply force
        }

        JPH::Vec3 normalizedDir = direction.Normalized();

        // Calculate yaw to look at player
        float yaw = atan2(normalizedDir.GetX(), normalizedDir.GetZ());
        trans->rotation = glm::vec3(0.0f, yaw, 0.0f); // Set rotation only around Y-axis

        float damping = 0.9f;

        JPH::Vec3 horizontalVelocity =
            JPH::Vec3(currentVelocity.GetX(), 0.0f, currentVelocity.GetZ()) * damping; // Dampen X and Z
        JPH::Vec3 newVelocity =
            JPH::Vec3(horizontalVelocity.GetX(), currentVelocity.GetY(), horizontalVelocity.GetZ()); // Keep Y unchanged

        // Apply force in the direction of the target
        bodyInterface.AddForce(enemyID, normalizedDir * enemy->moveSpeed);

        float currentSpeed = currentVelocity.Length();
        JPH::Vec3 clampedVelocity = JPH::Vec3(newVelocity.GetX(), -200.0f, newVelocity.GetZ());

        // Cap the body's speed
        if (currentSpeed > enemy->speedCap)
        {
            clampedVelocity *= (enemy->speedCap / currentSpeed);
        }

        JPH::Vec3 forward = JPH::Vec3(0, 0, 1);

        bodyInterface.SetLinearVelocity(enemyID, clampedVelocity);

        bodyInterface.SetRotation(enemyID, JPH::Quat::sIdentity(), JPH::EActivation::Activate);
    }
}

REGISTER_SYSTEM(StraySys);
