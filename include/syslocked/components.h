#pragma once

#include <glm/glm.hpp>
#include <vector>

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
};
