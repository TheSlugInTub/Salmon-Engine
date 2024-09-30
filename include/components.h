#include <ecs.h>
#include <glm/glm.hpp>
#include <model.h>
#include <string>
#include <physics.h>
#include <bone.h>
#include <animation.h>
#include <utils.h>

// Enum to specify what shape the collider is
enum ColliderType
{
    Box = 0, Sphere, Cylinder, Capsule
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
    
    RigidBody3D(ColliderType type, BodyState state, glm::vec3 size)
        : colliderType(type), boxSize(size), state(state)
    {}

    RigidBody3D(ColliderType type, BodyState state, float capRad, float capHeight)
        : colliderType(type), capsuleHeight(capHeight), capsuleRadius(capRad), state(state)
    {}

    RigidBody3D(ColliderType type, BodyState state, float sphereRadius)
        : colliderType(type),  sphereRadius(sphereRadius)
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

    Animator(Animation* animation)
        : currentAnimation(animation)
    {}
};

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
	anim->currentTime += anim->currentAnimation->GetTicksPerSecond() * dt;
	anim->currentTime = fmod(anim->currentTime, anim->currentAnimation->GetDuration());
	CalculateBoneTransform(&anim->currentAnimation->GetRootNode(), glm::mat4(1.0f), anim);
    }
}
