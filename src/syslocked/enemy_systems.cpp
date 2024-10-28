#include <syslocked/components.h>
#include <ecs.h>
#include <engine.h>
#include <input.h>
#include <physics.h>
#include <components.h>
#include <utils.h>

void StraySys()
{
    for (EntityID ent : SceneView<Stray>(engineState.scene))
    {
        auto trans = engineState.scene.Get<Transform>(ent);
        auto rigid = engineState.scene.Get<RigidBody3D>(ent);
        auto enemyID = rigid->body->GetID(); 

        trans->rotation = glm::vec3(6.25f, 0.0f, 0.0f);
        bodyInterface.SetRotation(enemyID, JPH::Quat::sIdentity(), JPH::EActivation::Activate);
        std::cout << "do it run\n";
    }
}

REGISTER_SYSTEM(StraySys);
