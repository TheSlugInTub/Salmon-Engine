#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <animation.h>
#include <physics.h>
#include <sound_source.h>
#include <sound_device.h>
#include <sound_buffer.h>

enum RigidbodyID
{
    RigidbodyID_None = 0,
    RigidbodyID_Bullet,
    RigidbodyID_Enemy,
    RigidbodyID_Player
};

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
    Model bulletModel;
    unsigned int bulletTexture;
    float bulletSpeed = 1.0f;

    SoundDevice* soundDevice;
    Sound shootSound;
    std::shared_ptr<SoundSource> soundSource;
};

struct Enemy
{
    int health;
    int registeredCollisionIndex = 0;
    bool isDead = false;

    SoundDevice* soundDevice;
    Sound hurtSound;
    std::shared_ptr<SoundSource> soundSource;
};

struct Stray
{
    float moveSpeed;
    float speedCap;
    Transform* playerTransform;
};

struct Bullet
{
    int randomValueThatWillGetReplaced;
};
