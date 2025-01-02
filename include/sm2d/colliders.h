#pragma once

#include <sm2d/types.h>

namespace sm2d
{

struct ColAABB
{
    glm::vec2 halfwidths;
};

struct ColPolygon
{
    std::vector<glm::vec2> points;      // Points in object space
    std::vector<glm::vec2> worldPoints; // Points in world space, get recomputed every frame
    glm::vec2              center;      // Geometric center
};

struct ColCircle
{
    float radius;
};

enum ColliderType
{
    sm2d_AABB,
    sm2d_Circle,
    sm2d_Polygon
};

struct Collider
{
    ColliderType type;
    ColAABB      aabb;
    ColCircle    circle;
    ColPolygon   polygon;
    Rigidbody*   body = nullptr;
    int          treeIndex = -1; // Index in the AABB tree

    int bodyIndex = 0; // Ignore this, it's only for the editor

    Collider(ColliderType type, const ColAABB& aabb, Rigidbody* body)
       : type(type), aabb(aabb), body(body)
    {
    }
    Collider(ColliderType type, const ColCircle& circle, Rigidbody* body)
       : type(type), circle(circle), body(body)
    {
    }
    Collider(ColliderType type, const ColPolygon& poly, Rigidbody* body)
       : type(type), polygon(poly), body(body)
    {
    }

    ~Collider() {} // This is just here so the compiler doesn't yell at me
    Collider() {}
};

struct Manifold
{
    bool      colliding;        // Are they colliding?
    glm::vec2 collisionNormal;  // Direction of the collision used for impulse calculation
    float     penetrationDepth; // How far they're inside each other
    glm::vec2 contactPoint;     // Point of contact
    Collider* objectA;          // Pointer to the first object involved in the collision
    Collider* objectB;          // Pointer to the second object involved in the collision

    operator bool() const { return colliding; }
};

// Intersection tests for the narrow phase

Manifold TestColAABBAABB(const Collider& a, const Collider& b);
Manifold TestColCircleCircle(const Collider& a, const Collider& b);
Manifold TestColPolygonPolygon(Collider& a, Collider& b);

Manifold TestColAABBCircle(const Collider& aabb, const Collider& circle);
Manifold TestColAABBPolygon(Collider& aabb, Collider& poly);
Manifold TestColCirclePolygon(const Collider& circle, const Collider& poly);

} // namespace sm2d
