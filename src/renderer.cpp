#include <renderer.h>
#include <model.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/common.hpp>
#include <engine.h>
#include <components.h>
#include <utils.h>
#include <stb_image.h>

namespace Renderer
{

void Init()
{
	Shader newShader("shaders/vertex.shad", "shaders/fragment.shad");
    defaultShader = newShader;

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);
}

void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view)
{
 	defaultShader.use();
	
    auto trans = engineState.scene.Get<Transform>(ent);
    auto model = engineState.scene.Get<MeshRenderer>(ent);

    glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform.
    transform = glm::translate(transform, trans->position);

    // Apply rotations around each axis (x, y, z) based on object.rotation
    transform = glm::rotate(transform, trans->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation around X-axis
    transform = glm::rotate(transform, trans->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation around Y-axis
    transform = glm::rotate(transform, trans->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotation around Z-axis

    transform = glm::scale(transform, trans->scale);

    // Setting all the uniforms.
	defaultShader.setMat4("model", transform);
	defaultShader.setMat4("view", view);
	defaultShader.setMat4("projection", projection);

    glBindTexture(GL_TEXTURE_2D, model->texture);
    model->model.Draw(defaultShader);
}

}
