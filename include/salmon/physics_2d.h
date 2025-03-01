#pragma once

#include "box2d/id.h"
#include <box2d/box2d.h>
#include <salmon/components.h>
#include <salmon/json.hpp>

enum Rigidbody2DType
{
    rg2d_Static,
    rg2d_Dynamic,
    rg2d_Kinematic
};

enum Collider2DType
{
    rg2d_Box,
    rg2d_Circle,
    rg2d_Polygon
};

struct Rigidbody2D
{
    Rigidbody2DType type;
    Transform*      transform = nullptr;

    float mass = 1.0f;
    float friction = 1.0f;
    float linearDamping = 0.98f;
    float angularDamping = 0.98f;
    float restitution = 0.0f;
    bool  fixedRotation = false;
    bool  alwaysAwake = false;
    int   userData = 0;

    b2BodyDef bodyDef;
    b2BodyId  bodyID;

    Rigidbody2D() {}

    ~Rigidbody2D() 
    {
        b2DestroyBody(bodyID);
        bodyID = b2_nullBodyId;
    }
};

struct Collider2D
{
    Collider2DType         colliderType;
    float                  radius;     // Circle
    glm::vec2              halfwidths; // Box
    std::vector<glm::vec2> points;     // Polygon

    b2ShapeDef shapeDef;
    b2ShapeId shapeID;
    b2Polygon  polygon;
    b2Circle   circle;

    Rigidbody2D* body = nullptr;

    Collider2D(float radius)
       : colliderType(rg2d_Circle), radius(radius)
    {
    }

    Collider2D(glm::vec2 halfwidths)
       : colliderType(rg2d_Box), halfwidths(halfwidths)
    {
    }

    Collider2D(const std::vector<glm::vec2>& points)
       : colliderType(rg2d_Box), points(points)
    {
    }

    Collider2D() {}

    ~Collider2D() 
    {
        b2DestroyShape(shapeID, false);
        shapeID = b2_nullShapeId;
    }
};

inline b2WorldDef worldDef;
inline b2WorldId  worldID;
inline b2DebugDraw debugDraw;

void InitPhysics2D();
void StepPhysics2D();
void DestroyPhysics2D();
