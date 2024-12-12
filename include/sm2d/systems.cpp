#include <sm2d/types.h>
#include <sm2d/colliders.h>
#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <sm2d/functions.h>
#include <glm/gtx/string_cast.hpp>
#include <salmon/clock.h>

namespace sm2d
{

void RigidbodySys()
{
    for (EntityID ent : SceneView<Rigidbody>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody>(ent);

        if (rigid->type == sm2d_Static || rigid->awake == false)
            continue;

        // rigid->force.y += -10.0f;

        rigid->lastPosition = glm::vec2(rigid->transform->position);

        rigid->linearVelocity += rigid->force / rigid->mass * engineState.deltaTime;
        rigid->linearVelocity *= glm::pow(rigid->linearDamping, engineState.deltaTime);
        rigid->transform->position.x += rigid->linearVelocity.x * engineState.deltaTime;
        rigid->transform->position.y += rigid->linearVelocity.y * engineState.deltaTime;

        rigid->angularVelocity += rigid->torque / rigid->mass * engineState.deltaTime;
        rigid->angularVelocity *= glm::pow(rigid->angularDamping, engineState.deltaTime);
        rigid->transform->rotation.z += rigid->angularVelocity * engineState.deltaTime;

        glm::vec2 diff = glm::vec2(rigid->transform->position) - rigid->lastPosition;
        if (glm::length(diff) > 0.0001f)
        {
            rigid->hasMoved = true;
        }
        else
        {
            rigid->hasMoved = false;
        }

        rigid->force = glm::vec2(0.0f);
        rigid->torque = 0.0f;
    }
}

void DebugSys()
{
    for (EntityID ent : SceneView<Collider>(engineState.scene))
    {
        auto collider = engineState.scene.Get<Collider>(ent);

        if (collider->type == ColliderType::sm2d_AABB)
        {
            glm::vec2 topLeft =
                glm::vec2(collider->body->transform->position) +
                glm::vec2(-collider->aabb.halfwidths.x, collider->aabb.halfwidths.y);
            glm::vec2 topRight =
                glm::vec2(collider->body->transform->position) +
                glm::vec2(collider->aabb.halfwidths.x, collider->aabb.halfwidths.y);
            glm::vec2 bottomRight =
                glm::vec2(collider->body->transform->position) +
                glm::vec2(collider->aabb.halfwidths.x, -collider->aabb.halfwidths.y);
            glm::vec2 bottomLeft =
                glm::vec2(collider->body->transform->position) +
                glm::vec2(-collider->aabb.halfwidths.x, -collider->aabb.halfwidths.y);

            std::vector<glm::vec3> points = {glm::vec3(bottomLeft, 0.0f), glm::vec3(topLeft, 0.0f),
                                             glm::vec3(topRight, 0.0f),
                                             glm::vec3(bottomRight, 0.0f)};

            Renderer::RenderLine(
                points, engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio()),
                engineState.camera->GetViewMatrix());
        }
    }
}

void ColliderSys()
{
    for (EntityID ent : SceneView<Collider>(engineState.scene))
    {
        auto collider = engineState.scene.Get<Collider>(ent);

        // if (!collider->body->hasMoved)
        //     continue;

        if (collider->type == ColliderType::sm2d_AABB)
        {
            RemoveLeaf(bvh, collider->treeIndex);
            RemoveDeletedLeaves(bvh);
            InsertLeaf(bvh, collider, ColAABBToABBB(*collider));
        }
        else if (collider->type == ColliderType::sm2d_Circle)
        {
            RemoveLeaf(bvh, collider->treeIndex);
            RemoveDeletedLeaves(bvh);
            InsertLeaf(bvh, collider, ColCircleToABBB(*collider));
        }
        else if (collider->type == ColliderType::sm2d_OBB)
        {
            RemoveLeaf(bvh, collider->treeIndex);
            RemoveDeletedLeaves(bvh);
            InsertLeaf(bvh, collider, ColOBBToAABB(*collider));
        }
    }
}

REGISTER_SYSTEM(RigidbodySys);
REGISTER_SYSTEM(ColliderSys);
// REGISTER_SYSTEM(DebugSys);

} // namespace sm2d
