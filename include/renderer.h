#include <shader.h>
#include <glm/glm.hpp>
#include <ecs.h>
#include <shader.h>

// 2D renderer for the engine
// Uses OpenGL to render quads

namespace Renderer
{

// Initializes OpenGL and intializes the VAO, VBO, and EBO
void Init();
// Renders an entity, gets its transform, and sprite renderer components
void Render(EntityID ent, const glm::mat4& view, const glm::mat4& projection);

inline unsigned int VAO, VBO, EBO;
inline Shader defaultShader;

}