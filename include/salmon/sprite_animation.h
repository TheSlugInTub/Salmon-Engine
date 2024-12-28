#pragma once

#include <glm/glm.hpp>
#include <salmon/components.h>

struct SpriteAnimation
{
    std::vector<unsigned int> textures;
    SpriteRenderer*           sprite;

    float speed; // Time between frames in seconds
    bool  playing = false;

    float timer;
    int currentSprite = 0; // Index into textures
};
