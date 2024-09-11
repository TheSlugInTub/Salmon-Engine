#include <ecs.h>
#include <glm/glm.hpp>
#include <model.h>
#include <string>
#include <physics.h>

// Enum to specify what shape the collider is
enum ColliderType
{
    Box = 0, Circle, Cylinder, Capsule
};

// Do it move?
enum BodyState
{
    Dynamic = 0, Static
};

// Describes the location and size of the entity in the world
struct Transform
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

// Component that describes how a mesh should be renderered at the transform of the entity
struct MeshRenderer
{
    Model model;
    glm::vec4 color = glm::vec4(1.0f);
    unsigned int texture;
    std::string texturePath = "";
};

// Component that simulates physics on the entity's transform with Jolt Physics
struct RigidBody3D
{
    ColliderType colliderType;
    BodyState state;
    glm::vec3 boxSize;
    float circleRadius = 1.0f;
    float capsuleRadius = 1.0f;
    float capsuleHeight = 2.0f;

    Body* body = nullptr;
    
    RigidBody3D(ColliderType type, BodyState state, glm::vec3 size)
        : colliderType(type), boxSize(size), state(state)
    {}

    RigidBody3D(ColliderType type, BodyState state, float capRad, float capHeight)
        : colliderType(type), capsuleHeight(capHeight), capsuleRadius(capRad), state(state)
    {}
};
