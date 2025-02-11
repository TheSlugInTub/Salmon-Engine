#pragma once

#include <vector>
#include <glm/glm.hpp>

struct IKSolver2D
{
    std::vector<glm::vec2> points = {};
    glm::vec2              endpoint = {};

    float pointDistance = 1.0f; // Distance between points

    IKSolver2D(const glm::vec2& base, const glm::vec2& endpointM,
               int size, float pointDistanceM)
    {
        endpoint = endpointM;
        pointDistance = pointDistanceM;

        points.resize(size);
        points[0] = base;
    }

    IKSolver2D() {}
};

void SolveIK2D(IKSolver2D& ik);
