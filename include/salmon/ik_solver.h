#pragma once

#include <vector>
#include <glm/glm.hpp>

struct IKSolver2D
{
    std::vector<glm::vec2> points = {};
    glm::vec2              endpoint = glm::vec2(0.0f, 0.0f);

    float pointDistance = 1.0f; // Distance between points
    int   numPoints = 5;

    IKSolver2D(const glm::vec2& base, const glm::vec2& endpointM,
               int size, float pointDistanceM)
    {
        endpoint = endpointM;
        pointDistance = pointDistanceM;
        numPoints = size;
        points.resize(numPoints);

        // Initialize points in a straight line from base
        for (int i = 0; i < numPoints; i++)
        {
            // Create initial chain going downward from base
            points[i] = base + glm::vec2(0.0f, -pointDistance * i);
        }
    }

    IKSolver2D() {}
};

void ResetIK2D(IKSolver2D& ik, const glm::vec2& dir);
void SolveIK2D(IKSolver2D& ik);
