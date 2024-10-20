#pragma once

#include <glm/glm.hpp>
#include <ecs.h>
#include <shader.h>
#include <vector>

inline unsigned int SHADOW_WIDTH = 2024, SHADOW_HEIGHT = 2024;

struct Transform;

// Struct to store the information about a light
struct Light
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec4 color = glm::vec4(1.0f);
    float radius = 1.0f;
    float innerRadius = 0.1f;
    float intensity = 1.0f;
    std::vector<glm::mat4> shadowTransforms;
    unsigned int depthMapFBO;
    unsigned int depthCubemap;
    bool castShadows = true;

    Light(glm::vec3 position, float radius, float innerRadius, float intensity, glm::vec4 color, bool castShadows = true)
        : position(position), radius(radius), innerRadius(innerRadius), intensity(intensity), color(color), castShadows(castShadows)
    {}
};

// 3D renderer for the engine
namespace Renderer
{

// Intializes OpenGL (only makes the shaders at the moment)
void Init();

// Makes a 4x4 matrix from a transform component
glm::mat4 MakeModelTransform(Transform* trans);

// Takes an entityID, gets its Transform and MeshRenderer components
// and uses the data to render it to the screen
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view);
// Renders a line from one vec3 to another vec3, uses the line shader
void RenderLine(glm::vec3 inPoint, glm::vec3 outPoint, const glm::mat4& projection, const glm::mat4& view);

// Default 3D shader
inline Shader defaultShader;
// Line shader, used for drawing 3d lines
inline Shader lineShader;
// Depth shader used for shadows and shadow mapping
inline Shader depthShader;

// All the lights in the scene
inline std::vector<Light> lights;

}
