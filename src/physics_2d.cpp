#include "box2d/box2d.h"
#include "box2d/math_functions.h"
#include "box2d/types.h"
#include <salmon/physics_2d.h>
#include <salmon/engine.h>

void InitPhysics2D()
{
    worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2(0.0f, -10.0f);

    worldID = b2CreateWorld(&worldDef);
}

void Rigidbody2DFixCollidersStartSys()
{
    for (EntityID ent : SceneView<Rigidbody2D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);
        auto col = engineState.scene.Get<Collider2D>(ent);
        auto trans = engineState.scene.Get<Transform>(ent);

        if (rigid->transform != nullptr)
        {
            rigid->transform = trans;
        }

        if (col->body != nullptr)
        {
            col->body = rigid;
        }
    }
}

void Rigidbody2DStartSys()
{
    for (EntityID ent : SceneView<Rigidbody2D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);

        rigid->bodyDef = b2DefaultBodyDef();

        rigid->bodyDef.position =
            b2Vec2(rigid->transform->position.x,
                   rigid->transform->position.y);

        if (rigid->type == rg2d_Dynamic)
        {
            rigid->bodyDef.type = b2_dynamicBody;
            rigid->bodyDef.angularDamping = rigid->angularDamping;
            rigid->bodyDef.linearDamping = rigid->linearDamping;
            rigid->bodyDef.fixedRotation = rigid->fixedRotation;
        }

        rigid->bodyID = b2CreateBody(worldID, &rigid->bodyDef);
    }
}

void Rigidbody2DSys()
{
    for (EntityID ent : SceneView<Rigidbody2D>(engineState.scene))
    {
        auto rigid = engineState.scene.Get<Rigidbody2D>(ent);

        b2Vec2 pos = b2Body_GetPosition(rigid->bodyID);
        b2Rot  rot = b2Body_GetRotation(rigid->bodyID);
        rigid->transform->position = glm::vec3(pos.x, pos.y, 0.0f);
        rigid->transform->rotation.z = b2Rot_GetAngle(rot);
    }
}

void Collider2DStartSys()
{
    for (EntityID ent : SceneView<Collider2D>(engineState.scene))
    {
        auto col = engineState.scene.Get<Collider2D>(ent);
        auto rigid = col->body;

        switch (col->colliderType)
        {
            case rg2d_Box:
            {
                col->polygon =
                    b2MakeBox(col->halfwidths.x, col->halfwidths.y);
                col->shapeDef = b2DefaultShapeDef();

                if (col->body->type == rg2d_Dynamic)
                {
                    col->shapeDef.friction = rigid->friction;
                    col->shapeDef.density = rigid->mass;
                    col->shapeDef.restitution = rigid->restitution;
                }

                b2CreatePolygonShape(col->body->bodyID,
                                     &col->shapeDef, &col->polygon);
                break;
            }
            case rg2d_Circle:
            {
                break;
            }
            case rg2d_Polygon:
            {
                break;
            }
        }
    }
}

REGISTER_START_SYSTEM(Rigidbody2DStartSys);
REGISTER_START_SYSTEM(Collider2DStartSys);

REGISTER_SYSTEM(Rigidbody2DSys);
