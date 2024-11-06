#include <random>
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

void EnemySpawnerSys()
{
    for (EntityID ent : SceneView<EnemySpawner>(engineState.scene))
    {
        auto spawner = engineState.scene.Get<EnemySpawner>(ent);

        spawner->spawnTimer -= engineState.deltaTime;

        if (spawner->spawnTimer <= 0)
        {
            // Spawn enemy
            glm::vec3 halfExtents = 0.5f * spawner->platformTrans->scale;

            float randomX = Utils::GenerateRandomNumber(spawner->platformTrans->position.x - halfExtents.x,
                                                        spawner->platformTrans->position.x + halfExtents.x);
            float randomZ = Utils::GenerateRandomNumber(spawner->platformTrans->position.z - halfExtents.z,
                                                        spawner->platformTrans->position.z + halfExtents.z);

            glm::vec3 randomPos = glm::vec3(randomX, 5.0f, randomZ);

            EntityID enemy = engineState.scene.AddEntity();
            engineState.scene.AssignParam<Transform>(enemy, randomPos, glm::vec3(2.5f, 0.0f, 0.0f),
                                         glm::vec3(3.03f, 3.03f, 3.03f));
            engineState.scene.AssignParam<MeshRenderer>(enemy, *spawner->enemyModel, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0);
            engineState.scene.AssignParam<RigidBody3D>(enemy, ColliderType::Capsule, BodyState::Dynamic, 1.0f, 4.0f,
                                           RigidbodyID_Enemy, glm::vec3(0.0f, -3.4f, 0.0f));
            engineState.scene.AssignParam<Enemy>(enemy, 15);
            engineState.scene.AssignParam<Stray>(enemy, 799110.0f * 2.0f, 10000000.0f, spawner->playerTrans);
            engineState.scene.AssignParam<Animator>(enemy, spawner->enemyRunAnim, true, true, 1.7f);

            RigidBody3DStartSys();
            EnemyStartSys();
            AnimatorStartSys();

            spawner->spawnTimer = spawner->timeBetweenSpawn;
        }
    }
}

REGISTER_SYSTEM(StraySys);
REGISTER_SYSTEM(EnemySpawnerSys);
