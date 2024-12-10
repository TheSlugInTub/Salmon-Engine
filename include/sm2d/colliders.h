#pragma once

#include <sm2d/types.h>

namespace sm2d
{

struct ColAABB
{
    glm::vec2 halfwidths;
};

struct ColOBB
{
    glm::vec2 halfwidths;
};

struct ColCircle
{
    float radius;
};

enum ColliderType
{
    sm2d_AABB,
    sm2d_Circle,
    sm2d_OBB
};

struct Collider
{
    ColliderType type;
    union
    {
        ColAABB   aabb;
        ColCircle circle;
        ColOBB    obb;
    };
    Rigidbody* body;
    int        treeIndex = -1; // Index in the AABB tree

    Collider(ColliderType type, const ColAABB& aabb, Rigidbody* body)
       : type(type), aabb(aabb), body(body)
    {
    }
    Collider(ColliderType type, const ColCircle& circle, Rigidbody* body)
       : type(type), circle(circle), body(body)
    {
    }
    Collider(ColliderType type, const ColOBB& obb, Rigidbody* body)
       : type(type), obb(obb), body(body)
    {
    }
};

struct CollisionData
{
    bool      colliding;
    glm::vec2 collisionNormal;  // Normal of the collision (direction of resolution)
    float     penetrationDepth; // Depth of penetration between the objects
    glm::vec2 contactPoint;     // Point of contact (optional, for accuracy in physics)
    Collider* objectA;          // Pointer to the first object involved in the collision
    Collider* objectB;          // Pointer to the second object involved in the collision

    operator bool() const { return colliding; }
};

// Intersection tests for the narrow phase

CollisionData TestColAABBAABB(const Collider& a, const Collider& b);
CollisionData TestColCircleCircle(const Collider& a, const Collider& b);
CollisionData TestColOBBOBB(const Collider& a, const Collider& b);

CollisionData TestColAABBCircle(const Collider& aabb, const Collider& circle);
CollisionData TestColAABBOBB(const Collider& aabb, const Collider& obb);

} // namespace sm2d
