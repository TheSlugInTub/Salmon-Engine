#include <sm2d/colliders.h>

namespace sm2d
{

CollisionData TestColAABB(const Collider& a, const Collider& b)
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
        result.contactPoint = glm::vec2(a.body->transform->position) + result.collisionNormal * a.aabb.halfwidths;

        // Include the objects in the collision data
        result.objectA = const_cast<Collider*>(&a);
        result.objectB = const_cast<Collider*>(&b);
    }

    return result;
}

} // namespace sm2d
