#pragma once

#include <vector>
#include <glm/glm.hpp>

typedef unsigned char Bool;

struct RopeSim;

void InitializeRope(RopeSim& rope);
void SimulateRope(RopeSim& rope);

struct RopeSim
{
    std::vector<glm::vec2> points = {};
    std::vector<glm::vec2> prevPoints = {};
    glm::vec2              basePosition = {};
    glm::vec2              endpoint = {};

    int numPoints = 5;

    float elasticity = 1.0f;
    float pointDistance = 1.0f;
    float damping = 0.98f;

    Bool useEndpoint = false;

    RopeSim(glm::vec2 basePosition, int numPoints, float elasticity, float pointDistance,
            float damping, glm::vec2 endpoint)
       : basePosition(basePosition), endpoint(endpoint), numPoints(numPoints), useEndpoint(true),
         elasticity(elasticity), pointDistance(pointDistance), damping(damping)
    {
        InitializeRope(*this);
    }

    RopeSim(glm::vec2 basePosition, int numPoints, float elasticity, float pointDistance,
            float damping)
       : basePosition(basePosition), useEndpoint(false), numPoints(numPoints),
         elasticity(elasticity), pointDistance(pointDistance), damping(damping)
    {
        InitializeRope(*this);
    }

    RopeSim() {}
};
