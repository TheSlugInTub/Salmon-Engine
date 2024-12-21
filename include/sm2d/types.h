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
    bool  awake; // Is the body's broadphase AABB gonna get updated or not? also skip the solver if
                 // this is false, this is set true when hasMoved is false and the collisions have
                 // transferred from last frame

    float linearDamping;  // Linear velocity gets exponentiated by this every frame
    float angularDamping; // Angular velocity gets exponentiated by this every frame

    float restitution = 0.0f; // Bounciness

    bool  fixedRotation = false; // If this is true, the body won't rotate
    float momentOfInertia;       // The closer to zero this is, the easier it is to be rotated

    void* userData = nullptr; // Put whatever you want in here, useful for marking tags

    glm::vec2 linearVelocity = glm::vec2(0.0f); // Rate of change of position
    float     angularVelocity = 0.0f;           // In radians

    glm::vec2 force = glm::vec2(0.0f); // Net force
    float     torque = 0.0f;           // In radians

    float gravityForce; // Force of gravity being applied to this object (DO NOT MODIFY OR TOUCH)
    bool  hasMoved;     // If it has moved in the last frame
};

struct Node
{
    int       index;
    Collider* collider;
    AABB      box;
    int       parentIndex;
    int       child1;
    int       child2;
    bool      leaf;
};

struct Tree
{
    std::vector<Node> nodes;
    int               rootIndex; // Index of the root node
};

inline Tree bvh;

} // namespace sm2d
