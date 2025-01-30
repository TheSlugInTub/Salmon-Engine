#pragma once

#include <glm/glm.hpp>
#include <sm2d/types.h>
#include <sm2d/colliders.h>

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
    glm::vec2 faceTarget = {};
    glm::vec2 bodyTarget = {};

    glm::vec2 legTarget[2] = {};
    glm::vec2 handTarget[2] = {};
    
    glm::vec2 legPos[2] = {};
    glm::vec2 handPos[2] = {};
    
    float circleCastRadius = 1.0f;
    
    // In local space
    glm::vec2 legRoot[2] = {glm::vec2(-0.3f, -0.4f), glm::vec2(0.3f, -0.4f)};
    glm::vec2 handRoot[2] = {glm::vec2(0.2f, 0.2f), glm::vec2(-0.2f, 0.2f)};

    sm2d::Collider* groundSensor[2];
};
