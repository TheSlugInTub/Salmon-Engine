#pragma once

#include <glm/glm.hpp>
#include <ecs.h>
#include <shader.h>

namespace Renderer
{

void Init();
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view);

inline Shader defaultShader;

}
