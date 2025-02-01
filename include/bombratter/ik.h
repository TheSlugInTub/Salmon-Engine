#pragma once

#include <glm/glm.hpp>
#include <sm2d/types.h>
#include <sm2d/colliders.h>
#include <salmon/rope.h>

#define GEN_INPUT_FIELD(inputText, target, stmt)                  \
    char##target##Buffer[128];                                    \
    strncpy_s(texBuffer, target.c_str(), sizeof(texBuffer));      \
    if (ImGui::InputText(inputText, texBuffer, sizeof(texBuffer), \
                         ImGuiInputTextFlags_EnterReturnsTrue))   \
    {                                                             \
        stmt                                                      \
    }

struct PlayerIK
{
    glm::vec2 legPos[2] = {};
    glm::vec2 handTarget[2] = {};

    float circleCastRadius = 1.0f;
    float legThreshold = 1.0f; // How far the leg can get away from the base

    float handElasticity = 1.0f;
    float handPointDistance = 1.0f;
    float handDamping = 0.98f;

    int handNumPoints = 5;

    // In local space
    glm::vec2 legRoot[2] = {glm::vec2(-0.3f, -0.4f), glm::vec2(0.3f, -0.4f)};
    glm::vec2 handRoot[2] = {glm::vec2(0.2f, 0.2f), glm::vec2(-0.2f, 0.2f)};

    sm2d::Collider* groundSensor[2];

    sm2d::Rigidbody* rigidbody = nullptr; // just so I don't have to get it every frame
    Transform*       transform = nullptr; // just so I don't have to get it every frame

    Transform* faceTransform; // Face transform
    Transform* bodyTransform; // Body transform

    RopeSim handRopeSim[2] = {}; // Rope simulations for the hands

    PlayerIK() {}
};
