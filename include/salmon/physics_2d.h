#pragma once

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

    b2BodyDef bodyDef;
    b2BodyId  bodyID = b2_nullBodyId;

    Rigidbody2D() {}
    Rigidbody2D(Rigidbody2DType type, Transform* transform,
                float mass, float friction, float linearDamping,
                float angularDamping, float restitution,
                bool fixedRotation, bool alwaysAwake, int userData)
       : type(type), transform(transform), mass(mass),
         friction(friction), linearDamping(linearDamping),
         angularDamping(angularDamping), restitution(restitution),
         fixedRotation(fixedRotation), alwaysAwake(alwaysAwake)
    {
    }
};

struct Collider2D
{
    Collider2DType         colliderType;
    float                  radius;     // Circle
    glm::vec2              halfwidths; // Box
    std::vector<glm::vec2> points;     // Polygon

    b2ShapeDef shapeDef;
    b2ShapeId  shapeID = b2_nullShapeId;
    b2Polygon  polygon;
    b2Circle   circle;

    Rigidbody2D* body = nullptr;

    uint32_t categoryBits = 0x00000000;
    uint32_t maskBits = 0xFFFFFFFF;

    Collider2D(Rigidbody2D* body, float radius,
               uint32_t categoryBits = 0x00000000,
               uint32_t maskBits = 0xFFFFFFFF)
       : colliderType(rg2d_Circle), radius(radius), body(body),
         categoryBits(categoryBits), maskBits(maskBits)
    {
    }

    Collider2D(Rigidbody2D* body, glm::vec2 halfwidths,
               uint32_t categoryBits = 0x00000000,
               uint32_t maskBits = 0xFFFFFFFF)
       : colliderType(rg2d_Box), halfwidths(halfwidths), body(body),
         categoryBits(categoryBits), maskBits(maskBits)
    {
    }

    Collider2D(Rigidbody2D*                  body,
               const std::vector<glm::vec2>& points,
               uint32_t categoryBits = 0x00000000,
               uint32_t maskBits = 0xFFFFFFFF)
       : colliderType(rg2d_Box), points(points), body(body),
         categoryBits(categoryBits), maskBits(maskBits)
    {
    }

    Collider2D() {}
};

enum Joint2DType
{
    rg2d_Revolute,
    rg2d_Distance
};

struct Joint2D
{
    Joint2DType type;
    b2BodyId    bodyA = b2_nullBodyId;
    b2BodyId    bodyB = b2_nullBodyId;

    glm::vec2 anchorA = glm::vec2(0.0f);
    glm::vec2 anchorB = glm::vec2(0.0f);

    float lowerAngle = 1.0f, upperAngle = 1.0f;
    float distance = 1.0f;

    union
    {
        b2RevoluteJointDef revoluteJointDef;
        b2DistanceJointDef distanceJointDef;
    };

    b2JointId jointID = b2_nullJointId;
};

struct ShapeCastContext2D
{
    b2BodyId  collidingBody = b2_nullBodyId;
    b2ShapeId collidingShape = b2_nullShapeId;
};

inline b2WorldDef  worldDef;
inline b2WorldId   worldID;
inline b2DebugDraw debugDraw;

void InitPhysics2D();
void StepPhysics2D();
void DestroyPhysics2D();

void Collider2DDebugSys();

void CreateRevoluteJoint(Joint2D& joint, const glm::vec2& anchorA,
                         const glm::vec2& anchorB, b2BodyId bodyA,
                         b2BodyId bodyB, float lowerAngle,
                         float upperAngle);

void CreateDistanceJoint(Joint2D& joint, const glm::vec2& anchorA,
                         const glm::vec2& anchorB, b2BodyId bodyA,
                         b2BodyId bodyB, float distance);

void Rigidbody2DStartSys();
void Collider2DStartSys();
void Rigidbody2DFixCollidersStartSys();

glm::vec2 b2ToGLM(b2Vec2 vec);
b2Vec2    b2ToGLM(glm::vec2 vec);
