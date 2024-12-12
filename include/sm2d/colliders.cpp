#include "functions.h"
#include <sm2d/colliders.h>
#include <cmath>
#include <array>

namespace sm2d
{

CollisionData TestColAABBAABB(const Collider& a, const Collider& b)
{
    CollisionData result = {};
    result.colliding = false;

    glm::vec2 diff = b.body->transform->position - a.body->transform->position;
    glm::vec2 combinedHalfWidths = a.aabb.halfwidths + b.aabb.halfwidths;

    // Calculate the overlap on each axis
    float overlapX = combinedHalfWidths.x - std::abs(diff.x);
    float overlapY = combinedHalfWidths.y - std::abs(diff.y);

    if (overlapX > 0 && overlapY > 0)
    {
        // There is a collision
        result.colliding = true;

        // Determine the axis of least penetration
        if (overlapX < overlapY)
        {
            result.penetrationDepth = overlapX;
            result.collisionNormal = glm::vec2(diff.x > 0 ? 1.0f : -1.0f, 0.0f);
        }
        else
        {
            result.penetrationDepth = overlapY;
            result.collisionNormal = glm::vec2(0.0f, diff.y > 0 ? 1.0f : -1.0f);
        }

        // Contact point (approximation: center of object A or B)
        result.contactPoint =
            glm::vec2(a.body->transform->position) + result.collisionNormal * a.aabb.halfwidths;

        // Include the objects in the collision data
        result.objectA = const_cast<Collider*>(&a);
        result.objectB = const_cast<Collider*>(&b);
    }

    return result;
}

CollisionData TestColCircleCircle(const Collider& a, const Collider& b)
{
    CollisionData collisionData;
    collisionData.colliding = false;

    // Vector between the two circle centers
    glm::vec2 delta = glm::vec2(b.body->transform->position - a.body->transform->position);
    float     distanceSquared = glm::dot(delta, delta);
    float     radiusSum = a.circle.radius + b.circle.radius;

    if (distanceSquared < radiusSum * radiusSum)
    {
        // Circles are colliding
        collisionData.colliding = true;

        float distance = std::sqrt(distanceSquared); // Actual distance between centers

        // Collision normal
        if (distance > 0.0f)
        {
            collisionData.collisionNormal = delta / distance;
        }
        else
        {
            // Circles are exactly on top of each other, arbitrary normal
            collisionData.collisionNormal = glm::vec2(1.0f, 0.0f);
        }

        // Penetration depth
        collisionData.penetrationDepth = radiusSum - distance;

        // Contact point
        collisionData.contactPoint = glm::vec2(a.body->transform->position) +
                                     collisionData.collisionNormal *
                                         (a.circle.radius - collisionData.penetrationDepth / 2.0f);

        // Include the objects in the collision data
        collisionData.objectA = const_cast<Collider*>(&a);
        collisionData.objectB = const_cast<Collider*>(&b);
    }
    else
    {
        // No collision
        collisionData.colliding = false;
    }

    return collisionData;
}

CollisionData TestColAABBCircle(const Collider& aabb, const Collider& circle)
{
    CollisionData result;
    result.colliding = false;

    glm::vec2 circleCenter = glm::vec2(circle.body->transform->position);
    glm::vec2 aabbCenter = glm::vec2(aabb.body->transform->position);

    // Find the closest point on the AABB to the circle's center
    glm::vec2 closestPoint;
    closestPoint.x = std::clamp(circleCenter.x, aabbCenter.x - aabb.aabb.halfwidths.x,
                                aabbCenter.x + aabb.aabb.halfwidths.x);

    closestPoint.y = std::clamp(circleCenter.y, aabbCenter.y - aabb.aabb.halfwidths.y,
                                aabbCenter.y + aabb.aabb.halfwidths.y);

    // Calculate the distance between the closest point and the circle's center
    glm::vec2 distanceVector = circleCenter - closestPoint;
    float     distanceSquared = glm::dot(distanceVector, distanceVector);

    // Check if collision occurs
    if (distanceSquared <= circle.circle.radius * circle.circle.radius)
    {
        result.colliding = true;

        // Calculate penetration depth
        float distance = std::sqrt(distanceSquared);
        result.penetrationDepth = circle.circle.radius - distance;

        // Calculate collision normal (from AABB's closest point to circle's center)
        if (distance > 0)
        {
            result.collisionNormal = glm::normalize(distanceVector);
        }
        else
        {
            // If circle's center is exactly at the AABB's center,
            // choose an arbitrary normal (e.g., pointing right)
            result.collisionNormal = glm::vec2(1.0f, 0.0f);
        }

        // Calculate contact point (closest point on AABB to circle's surface)
        result.contactPoint =
            closestPoint + result.collisionNormal * (distance > 0 ? distance : 0.0f);

        result.objectA = const_cast<Collider*>(&aabb);
        result.objectB = const_cast<Collider*>(&circle);
    }
    else
    {
        result.colliding = false;
        result.penetrationDepth = 0.0f;
        result.collisionNormal = glm::vec2(0.0f, 0.0f);
        result.contactPoint = closestPoint;
    }

    return result;
}

CollisionData TestColOBBOBB(const Collider& a, const Collider& b)
{
    CollisionData result = {};
    result.colliding = false;

    // Step 1: Calculate the rotation matrices for both OBBs
    float cos1 = std::cos(a.body->transform->rotation.z);
    float sin1 = std::sin(a.body->transform->rotation.z);
    float cos2 = std::cos(b.body->transform->rotation.z);
    float sin2 = std::sin(b.body->transform->rotation.z);

    glm::mat2 rot1(cos1, -sin1, sin1, cos1);
    glm::mat2 rot2(cos2, -sin2, sin2, cos2);

    glm::vec2 aPos = glm::vec2(a.body->transform->position);
    glm::vec2 bPos = glm::vec2(b.body->transform->position);

    // Step 2: Calculate the corners of both OBBs
    std::array<glm::vec2, 4> corners1 = {
        aPos + rot1 * glm::vec2(-a.obb.halfwidths.x, -a.obb.halfwidths.y),
        aPos + rot1 * glm::vec2(a.obb.halfwidths.x, -a.obb.halfwidths.y),
        aPos + rot1 * glm::vec2(a.obb.halfwidths.x, a.obb.halfwidths.y),
        aPos + rot1 * glm::vec2(-a.obb.halfwidths.x, a.obb.halfwidths.y)};

    std::array<glm::vec2, 4> corners2 = {
        bPos + rot2 * glm::vec2(-b.obb.halfwidths.x, -b.obb.halfwidths.y),
        bPos + rot2 * glm::vec2(b.obb.halfwidths.x, -b.obb.halfwidths.y),
        bPos + rot2 * glm::vec2(b.obb.halfwidths.x, b.obb.halfwidths.y),
        bPos + rot2 * glm::vec2(-b.obb.halfwidths.x, b.obb.halfwidths.y)};

    // Step 3: Calculate axes for Separating Axis Theorem (SAT)
    std::array<glm::vec2, 4> axes = {
        glm::normalize(rot1 * glm::vec2(1, 0)), // OBB1 X-axis
        glm::normalize(rot1 * glm::vec2(0, 1)), // OBB1 Y-axis
        glm::normalize(rot2 * glm::vec2(1, 0)), // OBB2 X-axis
        glm::normalize(rot2 * glm::vec2(0, 1))  // OBB2 Y-axis
    };

    float     minPenetration = std::numeric_limits<float>::max();
    glm::vec2 collisionAxis;

    // Step 4: SAT Collision Test
    for (const auto& axis : axes)
    {
        // Project corners onto the axis
        float minProj1 = std::numeric_limits<float>::max();
        float maxProj1 = std::numeric_limits<float>::lowest();
        float minProj2 = std::numeric_limits<float>::max();
        float maxProj2 = std::numeric_limits<float>::lowest();

        for (const auto& corner : corners1)
        {
            float proj = glm::dot(corner, axis);
            minProj1 = std::min(minProj1, proj);
            maxProj1 = std::max(maxProj1, proj);
        }

        for (const auto& corner : corners2)
        {
            float proj = glm::dot(corner, axis);
            minProj2 = std::min(minProj2, proj);
            maxProj2 = std::max(maxProj2, proj);
        }

        // Check for separation
        if (maxProj1 < minProj2 || maxProj2 < minProj1)
        {
            return result; // No collision
        }

        // Calculate penetration depth
        float penetration = std::min(maxProj1, maxProj2) - std::max(minProj1, minProj2);
        if (penetration < minPenetration)
        {
            minPenetration = penetration;
            collisionAxis = axis;
        }
    }

    result.objectA = const_cast<Collider*>(&a);
    result.objectB = const_cast<Collider*>(&b);

    // Step 5: Collision Resolution
    result.colliding = true;
    result.penetrationDepth = minPenetration;
    result.collisionNormal = collisionAxis;

    // Approximate contact point (center point between the OBBs)
    result.contactPoint = (aPos + bPos) * 0.5f;

    return result;
}

// struct ColAABB
// {
//     glm::vec2 halfwidths;
//     glm::vec2 position;
// };
// 
// struct ColOBB
// {
//     glm::vec2 halfwidths;
//     glm::vec2 position;
//     float rotation;
// };
// 
// struct CollisionData
// {
//     bool      colliding;
//     glm::vec2 collisionNormal;  // Normal of the collision (direction of resolution)
//     float     penetrationDepth; // Depth of penetration between the objects
//     glm::vec2 contactPoint;     // Point of contact (optional, for accuracy in physics)
//     Collider* objectA;          // Pointer to the first object involved in the collision
//     Collider* objectB;          // Pointer to the second object involved in the collision
// 
//     operator bool() const { return colliding; }
// };

CollisionData TestColAABBOBB(const Collider& aabb, const Collider& obb)
{
    CollisionData result;
    result.colliding = false;

    // Create rotation matrix for OBB
    float     cosTheta = std::cos(obb.body->transform->rotation.z);
    float     sinTheta = std::sin(obb.body->transform->rotation.z);
    glm::mat2 rotationMatrix = glm::mat2(cosTheta, -sinTheta, sinTheta, cosTheta);

    // Transform AABB vertices to OBB's local space
    glm::vec2 centerOffset =
        glm::vec2(aabb.body->transform->position - obb.body->transform->position);
    centerOffset = glm::transpose(rotationMatrix) * centerOffset;

    // AABB vertices in its local space
    glm::vec2 aabbVertices[4] = {glm::vec2(-aabb.aabb.halfwidths.x, -aabb.aabb.halfwidths.y),
                                 glm::vec2(aabb.aabb.halfwidths.x, -aabb.aabb.halfwidths.y),
                                 glm::vec2(aabb.aabb.halfwidths.x, aabb.aabb.halfwidths.y),
                                 glm::vec2(-aabb.aabb.halfwidths.x, aabb.aabb.halfwidths.y)};

    // Transform AABB vertices to OBB's local space
    for (auto& vertex : aabbVertices) { vertex = glm::transpose(rotationMatrix) * vertex; }

    // Axes to test (OBB local axes)
    glm::vec2 obbAxes[2] = {glm::vec2(1, 0), glm::vec2(0, 1)};

    float     minPenetration = std::numeric_limits<float>::max();
    glm::vec2 bestAxis;

    // Test separation on OBB local axes
    for (const auto& axis : obbAxes)
    {
        float minOBB = std::numeric_limits<float>::max();
        float maxOBB = std::numeric_limits<float>::lowest();
        float minAABB = std::numeric_limits<float>::max();
        float maxAABB = std::numeric_limits<float>::lowest();

        // Project OBB vertices
        for (float hw : {-obb.obb.halfwidths.x, obb.obb.halfwidths.x})
        {
            for (float hh : {-obb.obb.halfwidths.y, obb.obb.halfwidths.y})
            {
                glm::vec2 vertex(hw, hh);
                float     proj = glm::dot(vertex, axis);
                minOBB = std::min(minOBB, proj);
                maxOBB = std::max(maxOBB, proj);
            }
        }

        // Project AABB vertices
        for (const auto& vertex : aabbVertices)
        {
            float proj = glm::dot(vertex + centerOffset, axis);
            minAABB = std::min(minAABB, proj);
            maxAABB = std::max(maxAABB, proj);
        }

        // Check for separation
        if (maxOBB < minAABB || minOBB > maxAABB)
        {
            return result; // No collision
        }

        // Calculate penetration
        float penetration = std::min(maxOBB - minAABB, maxAABB - minOBB);
        if (penetration < minPenetration)
        {
            minPenetration = penetration;
            bestAxis = axis;
        }
    }

    // Collision detected
    result.colliding = true;
    result.penetrationDepth = minPenetration;

    // Determine collision normal in world space
    result.collisionNormal = rotationMatrix * bestAxis;
    if (glm::dot(result.collisionNormal, centerOffset) > 0)
    {
        result.collisionNormal = -result.collisionNormal;
    }

    result.objectA = const_cast<Collider*>(&obb);
    result.objectB = const_cast<Collider*>(&aabb);

    // Approximate contact point
    result.contactPoint = ClosestPointOnAABB(result.contactPoint, aabb);

    return result;
}

} // namespace sm2d
