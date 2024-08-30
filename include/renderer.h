#include <shader.h>
#include <glm/glm.hpp>
#include <ecs.h>
#include <shader.h>

namespace Renderer
{

void Init();
void Render(EntityID ent, const glm::mat4& view, const glm::mat4& projection);

inline unsigned int VAO, VBO, EBO;
inline Shader defaultShader;

}