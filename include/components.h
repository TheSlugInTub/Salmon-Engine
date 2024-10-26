#include <ecs.h>
#include <glm/glm.hpp>
#include <model.h>
#include <string>
#include <physics.h>
#include <bone.h>
#include <animation.h>
#include <utils.h>

// This file creates a bunch of core components that are neccessary for the engine to run

// Enum to specify what shape the collider is
enum ColliderType
{
    Box = 0, Sphere, Capsule, Mesh 
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
    glm::mat4 modelMat = glm::mat4(1.0f);
    bool useMatrix = false;
};

// Component that describes how a mesh should be renderered at the transform of the entity
struct MeshRenderer
{
    Model model;
    glm::vec4 color = glm::vec4(1.0f);
    unsigned int texture;
    std::string texturePath = "";
    unsigned int depthMapFBO = 0;
    unsigned int depthCubemap = 0;
};

// Component that simulates physics on the entity's transform with Jolt Physics
struct RigidBody3D
{
    ColliderType colliderType;
    BodyState state;
    glm::vec3 boxSize;
    float sphereRadius = 1.0f;
    float capsuleRadius = 1.0f;
    float capsuleHeight = 2.0f;

    Body* body = nullptr;
    int groupID = 0;

    RigidBody3D(ColliderType type, BodyState state, glm::vec3 size, int groupID = 0)
        : colliderType(type), boxSize(size), state(state), groupID(groupID)
    {}

    RigidBody3D(ColliderType type, BodyState state, float capRad, float capHeight, int groupID = 0)
        : colliderType(type), capsuleHeight(capHeight), capsuleRadius(capRad), state(state), groupID(groupID)
    {}

    RigidBody3D(ColliderType type, BodyState state, float sphereRadius, int groupID = 0)
        : colliderType(type), sphereRadius(sphereRadius), state(state), groupID(groupID)
    {}

    RigidBody3D(ColliderType type, BodyState state, int groupID = 0)
        : colliderType(type), state(state), groupID(groupID)
    {}
};

// Component that takes in an animation and plays it every frame
struct Animator
{
    bool playing = true;
    std::vector<glm::mat4> boneMatrices;
    Animation* currentAnimation;
    float currentTime;
    float deltaTime;
    bool looping = true;
    float speed = 1.0f;

    Animator(Animation* animation, bool playing = true, bool looping = true, float speed = 1.0f)
        : currentAnimation(animation), playing(playing), looping(looping), speed(speed)
    {}
};

// Forward declarations of systems just to make it so you can run them anytime which can be useful
void MeshRendererSys();
void RigidBody3DStartSys();
void RigidBody3DSys();

inline void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform, Animator* anim)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* Bone = anim->currentAnimation->FindBone(nodeName);

    if (Bone)
    {
	Bone->Update(anim->currentTime);
	nodeTransform = Bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;
 
    auto boneInfoMap = anim->currentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        anim->boneMatrices[index] = globalTransformation * offset;
    }    

    for (int i = 0; i < node->childrenCount; i++)
	CalculateBoneTransform(&node->children[i], globalTransformation, anim);
}

inline void UpdateAnimation(float dt, Animator* anim)
{
    anim->deltaTime = dt;
    if (anim->currentAnimation)
    {
        // Calculate the time increment
        anim->currentTime += anim->currentAnimation->GetTicksPerSecond() * dt;

        if (anim->looping) {
            // If looping, wrap the currentTime using modulo
            anim->currentTime = fmod(anim->currentTime, anim->currentAnimation->GetDuration());
        } else {
            // If not looping, check if we've reached the end of the animation
            if (anim->currentTime >= anim->currentAnimation->GetDuration()) {
                anim->currentTime = anim->currentAnimation->GetDuration(); // Clamp to end
                anim->playing = false;  // Animation has finished playing
                return;  // Exit the function, no need to update bone transforms 
            }
        }

        // Continue updating the bone transformations if still playing
        if (anim->playing) {
            CalculateBoneTransform(&anim->currentAnimation->GetRootNode(), glm::mat4(1.0f), anim);
        }
    }
}
