#pragma once

#include <salmon/ik_solver.h>
#include <glm/glm.hpp>
#include <sm2d/types.h>
#include <sm2d/colliders.h>
#include <salmon/rope.h>
#include <salmon/physics_2d.h>

#define GEN_INPUT_FIELD(inputText, target, stmt)                  \
    char##target##Buffer[128];                                    \
    strncpy_s(texBuffer, target.c_str(), sizeof(texBuffer));      \
    if (ImGui::InputText(inputText, texBuffer, sizeof(texBuffer), \
                         ImGuiInputTextFlags_EnterReturnsTrue))   \
    {                                                             \
        stmt                                                      \
    }

enum PlayerState : unsigned char
{
    Walking,
    Crawling
};

enum Category
{
    col_Static = 0x00000001,
    col_Player = 0x00000002,
    col_Enemy = 0x00000004,
    col_Item = 0x00000008
};

struct PlayerIK
{
    PlayerState state = PlayerState::Walking;

    Rigidbody2D* head = nullptr;
    Rigidbody2D* body[2] = {nullptr, nullptr};

    IKSolver2D legIK[2];
    IKSolver2D handIK[2];

    Collider2D* legCollider = nullptr;

    Collider2D* heldObjects[2];

    Joint2D revoluteJoints[3];
    Joint2D distanceJoints[2];

    SpriteRenderer* eyes;
    Transform*      eyesTransform;
    int             eyesTexture;
    int             closedEyesTexture;

    glm::vec2 legRoot[2] = {};
    glm::vec2 crawlingLegRoot[2] = {};
    glm::vec2 legTarget[2] = {};
    float     legLength = 1.0f;
    float     legThreshold = 0.5f;
    glm::vec2 handRoot[2] = {};
    glm::vec2 crawlingHandRoot[2] = {};
    float     handLength = 0.3f;
    bool      handHold[2] = {false, false}; // Is the hand holding something?

    float acceleration = 0.0f;
    float deceleration = 0.0f;
    float speed = 0.0f;
    float maxSpeed = 0.0f;
    float maxCrawlSpeed = 0.0f;
    float jumpSpeed = 200.0f;

    PlayerIK() {}
};

void PlayerIKSys();
