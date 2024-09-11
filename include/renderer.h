#pragma once

#include <glm/glm.hpp>
#include <ecs.h>
#include <shader.h>

// 3D renderer for the engine
namespace Renderer
{

// Intializes OpenGL (only makes the shaders at the moment)
void Init();

// Takes an entityID, gets its Transform and MeshRenderer components
// and uses the data to render it to the screen
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view);

// Renders a line from one vec3 to another vec3, uses the line shader
void RenderLine(glm::vec3 inPoint, glm::vec3 outPoint, const glm::mat4& projection, const glm::mat4& view);

// Default 3D shader
inline Shader defaultShader;
// Line shader, used for drawing 3d lines
inline Shader lineShader;

}
