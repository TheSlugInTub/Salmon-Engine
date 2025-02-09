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
    sm2d::Rigidbody* head = nullptr;
    sm2d::Rigidbody* body = nullptr;

    PlayerIK() {}
};
