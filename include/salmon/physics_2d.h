#pragma once

#include <box2d/box2d.h>
#include <salmon/components.h>

enum Rigidbody2DType
{
    rg2d_Dynamic,
    rg2d_Static,
    rg2d_Kinematic
};

enum Collider2DType
{
    rg2d_Box,
    rg2d_Polygon,
    rg2d_Circle
};

struct Rigidbody2D
{
    Rigidbody2DType type;
    Transform* transform;

    float mass = 1.0f;
    float friction = 1.0f;
    float linearDamping = 0.98f;
    float angularDamping = 0.98f;
    float restitution = 0.0f;
    bool fixedRotation = false;
    
    b2BodyDef bodyDef;
    b2BodyId  bodyID;
};

struct Collider2D
{
    Collider2DType         colliderType;
    float                  radius;     // Circle
    glm::vec2              halfwidths; // Box
    std::vector<glm::vec2> points;     // Polygon

    b2ShapeDef shapeDef;
    b2Polygon polygon;
    b2Circle  circle;

    Rigidbody2D* body;

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
};

inline b2WorldDef worldDef;
inline b2WorldId  worldID;

void InitPhysics2D();
