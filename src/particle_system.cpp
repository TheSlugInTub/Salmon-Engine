#include <salmon/particle_system.h>
#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <algorithm>
#include <glm/gtc/random.hpp>
#include <salmon/renderer.h>

void ParticleSystemSys()
{
    for (EntityID ent : SceneView<ParticleSystem>(engineState.scene))
    {
        auto par = engineState.scene.Get<ParticleSystem>(ent);

        if (!par->playing)
            continue;

        par->currentDuration += engineState.deltaTime;

        if (par->currentDuration < par->duration)
        {
            int particlesToSpawn = (int)(par->particleRate * engineState.deltaTime);
            for (int i = 0; i < particlesToSpawn && par->particles.size() < par->maxParticles; ++i)
            {
                Particle particle;
                particle.position = par->startingPosition;
                particle.rotation = par->startingRotation;
                particle.size = par->startingSize;
                particle.color = par->startingColor;
                particle.force =
                    (par->force + glm::vec3(glm::linearRand(-par->forceRandomness.x, par->forceRandomness.x),
                                            glm::linearRand(-par->forceRandomness.y, par->forceRandomness.y),
                                            glm::linearRand(-par->forceRandomness.z, par->forceRandomness.z))) *
                    par->forceMagnitude;
                particle.force = glm::normalize(particle.force);
                particle.lifetime = 0.0f;
                par->particles.push_back(particle);
            }
        }
        else if (par->currentDuration < par->duration)
        {
            if (par->looping)
            {
                par->currentDuration = 0.0f;
            }
            else
            {
                par->playing = false;
                par->currentDuration = 0.0f;
            }
        }

        for (Particle& particle : par->particles)
        {
            particle.forceMagnitude += par->forceOverTime;
            particle.position += (glm::normalize(particle.force) * particle.forceMagnitude) * engineState.deltaTime;
            particle.rotation += par->rotationOverTime * engineState.deltaTime;
            particle.size += par->sizeOverTime * engineState.deltaTime;
            particle.color += par->colorOverTime * engineState.deltaTime;
            particle.lifetime += engineState.deltaTime;
            particle.force += par->gravity;
            particle.forceMagnitude = par->forceMagnitude;
        }

        par->particles.erase(std::remove_if(par->particles.begin(), par->particles.end(), [&](const Particle& particle)
                                            { return particle.lifetime > par->particleLifetime; }),
                             par->particles.end());

        float aspectRatio = engineState.window->GetAspectRatio();
        Renderer::RenderParticleSystem(*par, engineState.camera->GetProjMatrix(aspectRatio),
                                       engineState.camera->GetViewMatrix());
    }
}
