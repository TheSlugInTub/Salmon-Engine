#include <salmon/ik_solver.h>
#include <salmon/editor.h>
#define GLM_ENABLE_EXPERIMENTAL

// Starting from the end of the point list, move the last point to the
// end point Calculate the ik.pointDistance between n and n-1 point,
// calculate the direction normalize direction, and put n-1 point in
// the direction + ik.pointDistance Flip the list and do it again

void SolveIK2D(IKSolver2D& ik)
{
    // Store the root position
    glm::vec2 rootPos = ik.points[0];

    // Forward pass - Move toward target
    for (int i = ik.points.size() - 1; i > 0; --i)
    {
        glm::vec2& point = ik.points[i];

        if (i == ik.points.size() - 1)
            point = ik.endpoint; // Set last point to target

        glm::vec2 direction =
            glm::normalize(ik.points[i - 1] - point);

        ik.points[i - 1] = point + (direction * ik.pointDistance);
    }

    // Backward pass - Maintain distances
    ik.points[0] = rootPos; // Restore root position

    for (int i = 0; i < ik.points.size() - 1; ++i)
    {
        glm::vec2& point = ik.points[i];
        glm::vec2  direction =
            glm::normalize(ik.points[i + 1] - point);

        ik.points[i + 1] = point + (direction * ik.pointDistance);
    }
}

void ResetIK2D(IKSolver2D& ik, const glm::vec2& dir)
{
    glm::vec2 base = ik.points[0];

    for (int i = 0; i < ik.numPoints; i++)
    {
        // Create initial chain going downward from base
        ik.points[i] = base + dir * ik.pointDistance * (float)i;
    }
}
