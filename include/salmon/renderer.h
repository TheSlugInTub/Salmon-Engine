#pragma once

// 3D renderer for the engine

#include <glm/glm.hpp>
#include <salmon/ecs.h>
#include <salmon/shader.h>
#include <vector>
#include <salmon/ui.h>
#include <salmon/tilemap.h>

// Resolution of the shadowDepthMap (cubemap which stores shadows)
// Increase if you want them to be higher quality
inline unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
// Amount of particles the renderer initiates in the OpenGL buffer
inline const int MAX_PARTICLES = 1000;
// Amount of tiles the renderer initiates in the OpenGL buffer
inline const int MAX_TILES = 1000;
inline const int MAX_UNIQUE_TILE_TEXTURES = 1000;

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
    bool                   castShadows = true;
    std::vector<glm::mat4> shadowTransforms;
    unsigned int           depthMapFBO;
    unsigned int           depthCubemap;

    Light(glm::vec3 position, float radius, float innerRadius, float intensity, glm::vec4 color,
          bool castShadows = true)
       : position(position), radius(radius), innerRadius(innerRadius), intensity(intensity),
         color(color), castShadows(castShadows)
    {
    }

    Light() {}
};

// 3D renderer for the engine
namespace Renderer
{

// OpenGL initialization functions
void Init(bool depth = true);
void InitShaders();
void Init2D();
void InitParticles();
void InitTilemaps();
void InitText();

// Makes a 4x4 matrix from a transform component
glm::mat4 MakeModelTransform(Transform* trans);

// Takes an entityID, gets its Transform and MeshRenderer components
// and uses the data to render it to the screen
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view);
// Renders a line from one vec3 to another vec3, uses the line shader
void RenderLine(const std::vector<glm::vec3>& points, const glm::mat4& projection,
                const glm::mat4& view, const glm::vec4& color = glm::vec4(0.0, 1.0f, 0.0f, 1.0f),
                float pointSize = 10.0f, float lineSize = 3.0f);
// Renders a line from one vec2 to another vec2, uses the line shader
void RenderLine2D(const std::vector<glm::vec2>& points, const glm::mat4& projection,
                const glm::mat4& view, const glm::vec4& color = glm::vec4(0.0, 1.0f, 0.0f, 1.0f),
                float pointSize = 10.0f, float lineSize = 3.0f, bool looping = true);
// Renders a single dot
void RenderPoint(const glm::vec3& point, const glm::mat4& projection, const glm::mat4& view,
                 const glm::vec4& color = glm::vec4(0.0, 1.0f, 0.0f, 1.0f));
// Takes an entityID, gets its Transform and SpriteRenderer components
// and uses the data to render it onto the screen
void RenderSprite(EntityID ent, const glm::mat4& projection, const glm::mat4& view);
// Renders all the particles of particle system instanced
void RenderParticleSystem(const ParticleSystem& par, const glm::mat4& projection,
                          const glm::mat4& view);
// Renders text
void RenderText(const Text& item, const glm::mat4& projection);
// Renders an orthographic quad
void RenderQuad(glm::vec2 position, glm::vec2 scale, float rotation, const glm::mat4& projection,
                unsigned int texture, const glm::vec4& color);
// Renders a tilemap
void RenderTilemap(const Tilemap& tilemap, const glm::mat4& projection, const glm::mat4& view);

// Default 3D shader
inline Shader defaultShader;
// Line shader, used for drawing 3d lines
inline Shader lineShader;
// Line shader, used for drawing 2d lines
inline Shader lineShader2d;
// Depth shader used for shadows and shadow mapping
inline Shader depthShader;
// Default 2D shader
inline Shader twoShader;
// Particle shader for instancing particles
inline Shader parShader;
// Text shader for rendering text
inline Shader textShader;
// Tilemap shader for instancing particles
inline Shader tileShader;

// All the lights in the scene
inline std::vector<Light> lights;

// OpenGL buffer objects for 2D rendering
inline unsigned int VAO, VBO, EBO;
// OpenGL buffer objects for text
inline unsigned int textVAO, textVBO;
// OpenGL buffer objects for line rendering   
inline GLuint lineVAO, lineVBO;
// OpenGL buffer objects for line rendering   
inline GLuint lineVAO2D, lineVBO2D;

// OpenGL buffer objects for 2d instanced tiles
inline unsigned int tileVBO, tileIndexVBO; // Can someone please find a better way of
                                           // rendering instanced tiles than having
                                           // two vbos for tile rendering
// OpenGL buffer objects for 2d instanced particles
inline unsigned int instancedVBO, instancedColorVBO; // Particles too
// Vector of matrices for the particles
inline std::vector<glm::mat4> particleMatrices;

} // namespace Renderer
