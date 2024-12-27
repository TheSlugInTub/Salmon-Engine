#pragma once

// 3D renderer for the engine

#include <glm/glm.hpp>
#include <salmon/ecs.h>
#include <salmon/shader.h>
#include <vector>
#include <salmon/ui.h>

// Resolution of the shadowDepthMap (cubemap which stores shadows)
// Increase if you want them to be higher quality
inline unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

struct Transform;
struct ParticleSystem;

// Struct to store the information about a light
struct Light
{
    glm::vec3              position = glm::vec3(0.0f);
    glm::vec4              color = glm::vec4(1.0f);
    float                  radius = 1.0f;
    float                  innerRadius = 0.1f;
    float                  intensity = 1.0f;
    std::vector<glm::mat4> shadowTransforms;
    unsigned int           depthMapFBO;
    unsigned int           depthCubemap;
    bool                   castShadows = true;

    Light(glm::vec3 position, float radius, float innerRadius, float intensity, glm::vec4 color,
          bool castShadows = true)
       : position(position), radius(radius), innerRadius(innerRadius), intensity(intensity),
         color(color), castShadows(castShadows)
    {
    }
};

// 3D renderer for the engine
namespace Renderer
{

// Intializes OpenGL
void Init(bool depth = true, bool ui = true);

// Makes a 4x4 matrix from a transform component
glm::mat4 MakeModelTransform(Transform* trans);

// Takes an entityID, gets its Transform and MeshRenderer components
// and uses the data to render it to the screen
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view);
// Renders a line from one vec3 to another vec3, uses the line shader
void RenderLine(const std::vector<glm::vec3>& points, const glm::mat4& projection,
                const glm::mat4& view, const glm::vec4& color = glm::vec4(0.0, 1.0f, 0.0f, 1.0f));
// Takes an entityID, gets its Transform and SpriteRenderer components
// and uses the data to render it onto the screen
void RenderSprite(EntityID ent, const glm::mat4& projection, const glm::mat4& view);
// Renders all the particles of particle system instanced
void RenderParticleSystem(const ParticleSystem& par, const glm::mat4& projection,
                          const glm::mat4& view);
// Renders text
void RenderText(const Text& item, const glm::mat4& projection);

void RenderQuad(glm::vec2 position, glm::vec2 scale, float rotation, const glm::mat4& projection,
                unsigned int texture, const glm::vec4& color);

// Default 3D shader
inline Shader defaultShader;
// Line shader, used for drawing 3d lines
inline Shader lineShader;
// Depth shader used for shadows and shadow mapping
inline Shader depthShader;
// Default 2D shader
inline Shader twoShader;
// Particle shader for instancing particles
inline Shader parShader;
// Text shader for rendering text
inline Shader textShader;

// All the lights in the scene
inline std::vector<Light> lights;

// OpenGL buffer objects for 2D rendering
inline unsigned int VAO, VBO, EBO;
// OpenGL buffer objects for text
inline unsigned int textVAO, textVBO;

// Max amount of particles the renderer can handle
inline const int MAX_PARTICLES = 1000;
// OpenGL buffer objects for 2d instanced particles
inline unsigned int instancedVBO, instancedColorVBO, instancedVAO;
// Vector of matrices for the particles
inline std::vector<glm::mat4> particleMatrices;

} // namespace Renderer
