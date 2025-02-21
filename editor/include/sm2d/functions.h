#pragma once

#include <sm2d/types.h>
#include <sm2d/colliders.h>
#include <optional>

namespace sm2d
{

// If the two lines intersect then return the intersection point
std::optional<glm::vec2> GetLineIntersection(const glm::vec2& p0, const glm::vec2& p1,
                                             const glm::vec2& q0, const glm::vec2& q1);

// Finds the closest point on a line segment to the vertex
glm::vec2 ClosestPointOnLineSegment(const glm::vec2& vertex, const glm::vec2& lineStart,
                                    const glm::vec2& lineEnd);

// Transforms a point from object space into world space
glm::vec2 LocalToWorld(glm::vec2 point, const glm::vec2 pos, float cosine, float sine);

// Updates a polygon's vertices to match its world space position and rotation
void UpdatePolygon(Collider& poly);

// Computes the geometric center of a polygon
glm::vec2 ComputePolygonCenter(ColPolygon& poly);

// Computes the top, down, left and right points of an AABB collider
void ComputeAABBPoints(const Collider& collider, std::vector<glm::vec2>& points);

// Cross product between vector and a scalar
glm::vec2 VectorScalarCross(const glm::vec2& v, float s);

// Enlarge AABB: a to encompass AABB: b
bool AABBEnlarge(AABB* a, const AABB& b);

// Returns an AABB that encompasses both a and b
AABB AABBUnion(const AABB& a, const AABB& b);

// Returns the surface area of an AABB
float AABBPerimeter(const AABB& a);

// Test to see if two AABBs overlap
bool AABBTest(const AABB& a, const AABB& b);

// Get the center of an AABB
glm::vec2 AABBCenter(const AABB& a);

// Calculates the closest point on the surface of the AABB to the glm::vec2 point
glm::vec2 ClosestPointOnAABB(const glm::vec2& point, const Collider& aabb);

// Returns the smaller float
float MinFloat(float a, float b);

// Returns the bigger float
float MaxFloat(float a, float b);

// Inserts a rigidbody into a tree
void InsertLeaf(Tree& tree, Collider* body, const AABB& box);

// Finds the best sibling for a new leaf
int FindBestSibling(Tree& tree, const AABB& box);

// Removes a rigidbody from a tree
void RemoveLeaf(Tree& tree, int leafIndex);

// Removes all the marked leaves from the tree's vector of nodes
void RemoveDeletedLeaves(Tree& tree);

// Traverses through a tree and detects all the collisions and puts them in collisionResults
void GetCollisionsInTree(const Tree& tree, std::vector<Manifold>& collisionResults);

// Resolves all collisions based on the given ColiisionData
void ResolveCollisions(const Tree& tree, std::vector<Manifold>& collisionResults);

// Returns the 2d cross product of two vectors
float CrossProduct(const glm::vec2& a, const glm::vec2& b);

// Update a collider, does the same as the ColliderSys
void UpdateCollider(Collider* collider);

// Conversion functions

AABB ColAABBToABBB(const Collider& box);      // Returns bounding box encapsulating an AABB collider
AABB ColCircleToABBB(const Collider& circle); // Returns bounding box encapsulating a Circle
AABB ColPolygonToAABB(
    const Collider& poly); // Returns bounding box encapsulating a Polygon collider

} // namespace sm2d
