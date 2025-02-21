#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Particle
{
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 size;
    glm::vec4 color;
    glm::vec3 force;
    float     forceMagnitude;
    float     lifetime;
};

struct ParticleSystem
{
    unsigned int texture;          // Texture of all particles
    glm::vec3    startingPosition; // Starting position of all particles
    glm::vec3    startingRotation; // Starting rotation of all particles
    glm::vec3    startingSize;     // Starting size of all particles

    float     sizeOverTime;     // The particle size over time, positive = growing, negative = shrinking
    glm::vec3 rotationOverTime; // The amount of rotation added to each particle per frame

    glm::vec3 force;          // The force applied on each particle (normalized)
    float     forceMagnitude; // The magnitude of the force, this is here so that randomizing force doesn't change its
                              // magnitude
    glm::vec3 forceRandomness;    // How randomized the force is, only affects direction
    float forceOverTime;      // The magnitude of the force over time, positive = force grows, negative = force shrinks

    glm::vec4 startingColor; // Starting color of all particles
    glm::vec4 colorOverTime; // The amount of color added to each particle per frame

    glm::vec3 gravity; // Gravity of the particles

    float particleLifetime; // The amount of time, a particle can last before dying
    float duration;         // The amount of time, the system can produce particles
    float currentDuration; // The current duration, currentDuration += deltaTime, gets reset once it has reached the end
                           // duration
    float        particleRate; // The rate of particles
    unsigned int maxParticles; // The maximum amount of particles that the system can have
    bool         looping;      // Does the system loop after it's reached its max duration?
    bool         playing;      // Determines whether the system is currently playing right now
    bool         billboarded;  // Determines whether the sprites are billboarded or not

    std::vector<Particle> particles;
    std::string texturePath;
};

void ParticleSystemSys();
