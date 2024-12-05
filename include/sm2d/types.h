#pragma once

#include <glm/glm.hpp>
#include <salmon/components.h>

#define SM_PI (3.14159265359f)

namespace sm2d
{

struct Collider;

struct AABB
{
    glm::vec2 upperBound; // Top right
    glm::vec2 lowerBound; // Bottom left
};

struct Circle
{
    glm::vec2 center; // The center of the circle
    float     radius; // The radius of the circle
};

enum BodyType
{
    sm2d_Static = 0,
    sm2d_Dynamic = 1,
    sm2d_Kinematic = 2
};

struct Rigidbody
{
    BodyType type;

    Transform* transform;

    float mass;
    bool  awake;

    float linearDamping;
    float angularDamping;

    glm::vec2 linearVelocity;
    float     angularVelocity; // In radians

    void* userData;
    bool  fixedRotation; // If this is true, the body won't rotate

    glm::vec2 force;
    float     torque; // In radians

    bool hasMoved; // Indicates if it has moved in the last frame
    glm::vec2 lastPosition;
};

struct Node
{
    int index;
    Collider* collider; 
    AABB box;
    int parentIndex;
    int child1;
    int child2;
    bool leaf;
};

struct Tree
{
    std::vector<Node> nodes;
    int rootIndex; // Index of the root node
};

inline Tree bvh;

} // namespace sm2d
