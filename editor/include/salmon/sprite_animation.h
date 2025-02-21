#pragma once

#include <glm/glm.hpp>
#include <salmon/components.h>

struct SpriteAnimation
{
    std::vector<unsigned int> textures;
    float                     speed; // Time between frames in seconds
    std::string               name;

    bool operator==(const SpriteAnimation& anim) { return name == anim.name; }
};

struct SpriteAnimator
{
    SpriteRenderer*              sprite;
    std::vector<SpriteAnimation> animations;

    bool playing = false;

    float timer; // When this reaches zero, it's time for the next frame and gets reset to the
                 // current frame's speed
    int              currentSprite = 0; // Index into textures
    SpriteAnimation* currentAnim = nullptr;
};

void PlaySpriteAnimation(SpriteAnimator* spriteAnim, const std::string& name);
