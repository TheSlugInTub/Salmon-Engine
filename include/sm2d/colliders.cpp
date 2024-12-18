#include <salmon/renderer.h>
#include <sm2d/functions.h>
#include <sm2d/colliders.h>
#include <cmath>
#include <array>
#include <cassert>
#include <glm/gtx/string_cast.hpp>
#include <limits>

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

CollisionData TestColPolygonPolygon(Collider& a, Collider& b)
{
    CollisionData result = {};
    result.colliding = false;
    Collider* col1 = &a;
    Collider* col2 = &b;

    float     minOverlap = std::numeric_limits<float>::max();
    glm::vec2 minAxis;

    if (col1 == col2)
    {
        return result;
    }

    for (int shape = 0; shape < 2; ++shape)
    {
        if (shape == 1)
        {
            col1 = &b;
            col2 = &a;
        }

        for (int a2 = 0; a2 < col1->polygon.worldPoints.size(); a2++)
        {
            int b = (a2 + 1) % col1->polygon.worldPoints.size();

            glm::vec2 axisProj = {
                -(col1->polygon.worldPoints[b].y - col1->polygon.worldPoints[a2].y),
                col1->polygon.worldPoints[b].x - col1->polygon.worldPoints[a2].x};
            axisProj = glm::normalize(axisProj);

            float min1 = INFINITY;
            float max1 = -INFINITY;
            for (int p = 0; p < col1->polygon.worldPoints.size(); ++p)
            {
                float q = glm::dot(col1->polygon.worldPoints[p], axisProj);
                min1 = std::min(min1, q);
                max1 = std::max(max1, q);
            }

            float min2 = INFINITY;
            float max2 = -INFINITY;
            for (int p = 0; p < col2->polygon.worldPoints.size(); ++p)
            {
                float q = glm::dot(col2->polygon.worldPoints[p], axisProj);
                min2 = std::min(min2, q);
                max2 = std::max(max2, q);
            }

            // Calculate overlap
            float overlap = std::min(max1, max2) - std::max(min1, min2);

            // Track the smallest overlap and corresponding axis
            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                minAxis = axisProj;
            }

            if (!(max2 >= min1 && max1 >= min2))
                return result;
        }
    }

    result.colliding = true;
    result.objectA = &a;
    result.objectB = &b;
    result.penetrationDepth = minOverlap;

    // Determine contact point (simplified method)
    result.contactPoint = glm::vec2(0.0f);
    size_t contactPointCount = 0;
    for (const auto& pointA : a.polygon.worldPoints)
    {
        for (const auto& pointB : b.polygon.worldPoints)
        {
            if (glm::distance(pointA, pointB) < 0.001f) // Close points are considered contact
            {
                result.contactPoint += pointA;
                contactPointCount++;
            }
        }
    }

    // Average contact points if multiple found
    if (contactPointCount > 0)
    {
        result.contactPoint /= static_cast<float>(contactPointCount);
    }
    else
    {
        // If no exact contact points, use center point between the two polygons
        result.contactPoint = (a.polygon.center + b.polygon.center) * 0.5f;
    }

    Renderer::RenderLine(
        {glm::vec3(result.contactPoint, 0.0f), glm::vec3(result.contactPoint, 0.0f)},
        engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio()),
        engineState.camera->GetViewMatrix());

    // Ensure normal points from A to B
    result.collisionNormal = minAxis;
    if (glm::dot(result.collisionNormal, b.polygon.center - a.polygon.center) < 0)
    {
        result.collisionNormal = -result.collisionNormal;
    }

    return result;
}

CollisionData TestColAABBPolygon(Collider& aabb, Collider& poly)
{
    CollisionData result = {};
    result.colliding = false;

    ColPolygon a;
    a.center = glm::vec2(aabb.body->transform->position);
    ComputeAABBPoints(aabb, a.worldPoints);
    ColPolygon* col1 = &a;
    ColPolygon* col2 = &poly.polygon;

    float     minOverlap = std::numeric_limits<float>::max();
    glm::vec2 minAxis;

    if (col1 == col2)
    {
        return result;
    }

    for (int shape = 0; shape < 2; ++shape)
    {
        if (shape == 1)
        {
            col1 = &poly.polygon;
            col2 = &a;
        }

        for (int a2 = 0; a2 < col1->worldPoints.size(); a2++)
        {
            int b = (a2 + 1) % col1->worldPoints.size();

            glm::vec2 axisProj = {-(col1->worldPoints[b].y - col1->worldPoints[a2].y),
                                  col1->worldPoints[b].x - col1->worldPoints[a2].x};

            float min1 = INFINITY;
            float max1 = -INFINITY;
            for (int p = 0; p < col1->worldPoints.size(); ++p)
            {
                float q = glm::dot(col1->worldPoints[p], axisProj);
                min1 = std::min(min1, q);
                max1 = std::max(max1, q);
            }

            float min2 = INFINITY;
            float max2 = -INFINITY;
            for (int p = 0; p < col2->worldPoints.size(); ++p)
            {
                float q = glm::dot(col2->worldPoints[p], axisProj);
                min2 = std::min(min2, q);
                max2 = std::max(max2, q);
            }

            // Calculate overlap
            float overlap = std::min(max1, max2) - std::max(min1, min2);

            // Track the smallest overlap and corresponding axis
            if (overlap < minOverlap)
            {
                minOverlap = overlap;
                minAxis = axisProj;
            }

            if (!(max2 >= min1 && max1 >= min2))
                return result;
        }
    }

    result.colliding = true;
    result.objectA = &aabb;
    result.objectB = &poly;
    result.penetrationDepth = minOverlap;

    // Determine contact point (simplified method)
    result.contactPoint = glm::vec2(0.0f);
    size_t contactPointCount = 0;
    for (const auto& pointA : a.worldPoints)
    {
        for (const auto& pointB : poly.polygon.worldPoints)
        {
            if (glm::distance(pointA, pointB) < 0.001f) // Close points are considered contact
            {
                result.contactPoint += pointA;
                contactPointCount++;
            }
        }
    }

    // Average contact points if multiple found
    if (contactPointCount > 0)
    {
        result.contactPoint /= static_cast<float>(contactPointCount);
    }
    else
    {
        // If no exact contact points, use center point between the two polygons
        result.contactPoint = (a.center + poly.polygon.center) * 0.5f;
    }

    // Ensure normal points from A to B
    result.collisionNormal = minAxis;
    if (glm::dot(result.collisionNormal, poly.polygon.center - a.center) < 0)
    {
        result.collisionNormal = -result.collisionNormal;
    }

    return result;
}

CollisionData TestColCirclePolygon(const Collider& circle, const Collider& poly)
{
    // FIXME
    CollisionData result;
    result.colliding = false;

    return result;
}

} // namespace sm2d
