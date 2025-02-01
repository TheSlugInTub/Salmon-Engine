#include <salmon/rope.h>
#include <salmon/engine.h>

void InitializeRope(RopeSim& rope)
{
    rope.points.resize(rope.numPoints);
    rope.prevPoints.resize(rope.numPoints);

    for (size_t i = 0; i < rope.points.size(); ++i)
    {
        rope.points[i] =
            rope.basePosition + glm::vec2(0, -static_cast<float>(i) * rope.pointDistance);
        rope.prevPoints[i] = rope.points[i];
    }
}

void SimulateRope(RopeSim& rope)
{
    // Update the position of the base point
    rope.points[0] = rope.basePosition;

    for (size_t i = 1; i < rope.points.size(); ++i)
    {
        glm::vec2& point = rope.points[i];
        glm::vec2& prevPoint = rope.prevPoints[i];

        // Verlet integration
        glm::vec2 temp = point;
        glm::vec2 gravity(
            0, -9.81f * (1.0f - (rope.elasticity * static_cast<float>(i) / rope.points.size())));
        point += (point - prevPoint) * rope.damping +
                 gravity * engineState.deltaTime * engineState.deltaTime;
        prevPoint = temp;

        // Constrain point to be within the rope's length from the previous point
        glm::vec2& previPoint = rope.points[i - 1];

        glm::vec2 direction = point - prevPoint;
        float     distance = glm::length(direction);

        if (distance > rope.pointDistance)
        {
            glm::vec2 correction = direction * ((distance - rope.pointDistance) / distance);
            point -= correction * 0.5f;
            previPoint += correction * 0.5f;
        }
    }

    // Constrain the last point to the endpoint if useEndPoint is true
    if (rope.useEndpoint)
    {
        rope.points.back() = rope.endpoint;
        rope.prevPoints.back() = rope.endpoint;
    }
}
