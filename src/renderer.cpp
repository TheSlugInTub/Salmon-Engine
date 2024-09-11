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
    Shader newLineShader("shaders/linevertex.shad", "shaders/linefragment.shad");
    lineShader = newLineShader;

    stbi_set_flip_vertically_on_load(true);

    // Enable the DEPTH_TEST, basically just so faces don't draw on top of eachother in weird ways
    glEnable(GL_DEPTH_TEST);
}

void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view)
{
    // Activate the shader program
 	defaultShader.use();
	
    // Gets the components of the entity
    auto trans = engineState.scene.Get<Transform>(ent);
    auto model = engineState.scene.Get<MeshRenderer>(ent);

    glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform.
    transform = glm::translate(transform, trans->position);

    // Apply rotations around each axis (x, y, z) based on trans->rotation
    transform = glm::rotate(transform, trans->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation around X-axis
    transform = glm::rotate(transform, trans->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation around Y-axis
    transform = glm::rotate(transform, trans->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotation around Z-axis

    // Scales the transform
    transform = glm::scale(transform, trans->scale);

    // Setting all the uniforms.
	defaultShader.setMat4("model", transform);
	defaultShader.setMat4("view", view);
	defaultShader.setMat4("projection", projection);

    // Binds the texture of the MeshRenderer component
    glBindTexture(GL_TEXTURE_2D, model->texture);

    // Draws the model
    model->model.Draw(defaultShader);
}

void RenderLine(glm::vec3 inPoint, glm::vec3 outPoint, const glm::mat4& projection, const glm::mat4& view)
{
    std::vector<glm::vec3> points = {inPoint, outPoint};

    GLuint lVAO, lVBO;
    glGenVertexArrays(1, &lVAO);
    glGenBuffers(1, &lVBO);

    glBindVertexArray(lVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lVBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    lineShader.use();

    lineShader.setMat4("view", view);
    lineShader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    lineShader.setMat4("model", model);

    // Draw line
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, points.size());

    // Draw points
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, points.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &lVBO);
    glDeleteVertexArrays(1, &lVAO);
}

}
