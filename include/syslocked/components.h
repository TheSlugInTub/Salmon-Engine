#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <animation.h>

struct PlayerMovement
{
    float speed;
    float jumpSpeed;
};

struct Gun
{
    float forwardOffset;
    float rightOffset;
    float upOffset;
    Animation* gunShootAnim;
    float gunTimer = 1.0f;
};
