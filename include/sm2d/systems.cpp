#include <sm2d/types.h>
#include <sm2d/colliders.h>
#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <sm2d/functions.h>
#include <glm/gtx/string_cast.hpp>
#include <salmon/clock.h>

namespace sm2d
{

void RigidbodyStartSys()
{
    for (EntityID ent : SceneView<Rigidbody>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody>(ent);

        if (rigid->transform == nullptr)
        {
            if (auto trans = engineState.scene.Get<Transform>(ent))
            {
                rigid->transform = trans;
            }
        }
    }
}

void RigidbodySys()
{
    for (EntityID ent : SceneView<Rigidbody>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody>(ent);

        if (rigid->type == sm2d_Static || !rigid->awake)
        {
            continue;
        }

        rigid->force.y += -3.5f * rigid->mass; // GRAVITAS

        rigid->linearVelocity += rigid->force / rigid->mass * engineState.deltaTime;
        rigid->linearVelocity *= glm::pow(rigid->linearDamping, engineState.deltaTime);
        rigid->transform->position.x += rigid->linearVelocity.x * engineState.deltaTime;
        rigid->transform->position.y += rigid->linearVelocity.y * engineState.deltaTime;

        rigid->angularVelocity += rigid->torque / rigid->mass * engineState.deltaTime;
        rigid->angularVelocity *= glm::pow(rigid->angularDamping, engineState.deltaTime);
        rigid->transform->rotation.z += rigid->angularVelocity * engineState.deltaTime;

        if (rigid->angularVelocity > 0.05f || glm::length(rigid->linearVelocity) > 0.01f)
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

// PLEASE TURN THIS OFF WHEN YOU CAN, THIS FUNCTION IS AN ABSOLUTE CATASTROPHE
void DebugSys()
{
    for (EntityID ent : SceneView<Collider>(engineState.scene))
    {
        auto collider = engineState.scene.Get<Collider>(ent);

        if (collider->body == nullptr)
        {
            if (auto rigid = engineState.scene.Get<Rigidbody>(ent))
            {
                collider->body = rigid;

                if (rigid->transform == nullptr)
                {
                    if (auto trans = engineState.scene.Get<Transform>(ent))
                    {
                        rigid->transform = trans;
                    }
                }
            }
        }

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
        else if (collider->type == ColliderType::sm2d_Polygon)
        {
            UpdatePolygon(*collider);
            std::vector<glm::vec3> threedpoints;
            for (auto& point : collider->polygon.worldPoints)
            {
                threedpoints.push_back(glm::vec3(point, 0.0f));
            }
            Renderer::RenderLine(
                threedpoints,
                engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio()),
                engineState.camera->GetViewMatrix());

            Renderer::RenderLine(
                {glm::vec3(collider->polygon.center, 0.0f),
                 glm::vec3(collider->polygon.center, 0.0f)},
                engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio()),
                engineState.camera->GetViewMatrix());
        }
    }
}

void ColliderStartSys()
{
    for (EntityID ent : SceneView<Collider>(engineState.scene))
    {
        auto collider = engineState.scene.Get<Collider>(ent);

        if (collider->type == ColliderType::sm2d_AABB)
        {
            InsertLeaf(bvh, collider, ColAABBToABBB(*collider));
        }
        else if (collider->type == ColliderType::sm2d_Circle)
        {
            InsertLeaf(bvh, collider, ColCircleToABBB(*collider));
        }
        else if (collider->type == ColliderType::sm2d_Polygon)
        {
            for (int i = 0; i < collider->polygon.points.size(); ++i)
            {
                collider->polygon.worldPoints.push_back(glm::vec2(0.0f, 0.0f));
            }
            UpdatePolygon(*collider);
            collider->polygon.center = ComputePolygonCenter(collider->polygon);
            InsertLeaf(bvh, collider, ColPolygonToAABB(*collider));
        }
    }
}

void ColliderSys()
{
    for (EntityID ent : SceneView<Collider>(engineState.scene))
    {
        auto collider = engineState.scene.Get<Collider>(ent);

        if (collider->body == nullptr)
        {
            if (auto bod = engineState.scene.Get<Rigidbody>(ent))
            {
                collider->body = bod;
            }
        }

        if (collider->body->type == BodyType::sm2d_Static || !collider->body->awake)
        {
            continue;
        }

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
        else if (collider->type == ColliderType::sm2d_Polygon)
        {
            UpdatePolygon(*collider);
            collider->polygon.center = ComputePolygonCenter(collider->polygon);
            RemoveLeaf(bvh, collider->treeIndex);
            RemoveDeletedLeaves(bvh);
            InsertLeaf(bvh, collider, ColPolygonToAABB(*collider));
        }
    }
}

REGISTER_START_SYSTEM(ColliderStartSys);
REGISTER_START_SYSTEM(RigidbodyStartSys);

REGISTER_SYSTEM(RigidbodySys);
REGISTER_SYSTEM(ColliderSys);
REGISTER_EDITOR_SYSTEM(DebugSys);

} // namespace sm2d
