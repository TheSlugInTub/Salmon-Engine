#pragma once

#include <sm2d/types.h>
#include <sm2d/colliders.h>

namespace sm2d
{

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
void GetCollisionsInTree(const Tree& tree, std::vector<CollisionData>& collisionResults);

// Resolves all collisions based on the given ColiisionData
void ResolveCollisions(const Tree& tree, std::vector<CollisionData>& collisionResults);

// Conversion functions

AABB ColAABBToABBB(const Collider& box);

} // namespace sm2d
