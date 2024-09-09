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

void RenderLine(glm::vec3 inPoint, glm::vec3 outPoint)
{
    glm::mat4 projection = engineState.camera->GetProjMatrix(engineState.window->GetAspectRatio());
    glm::mat4 view = engineState.camera->GetViewMatrix();

    std::vector<glm::vec3> vertices;

    vertices.push_back(inPoint);
    vertices.push_back(outPoint);
    
    // Create and bind VAO and VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    // Define vertex attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    // Use the shader program
    lineShader.use();

    // Set shader uniforms
    lineShader.setMat4("view", view);
    lineShader.setMat4("projection", projection);
    lineShader.setVec4("lineColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    glm::mat4 model = glm::mat4(1.0f);
    lineShader.setMat4("model", model);

    // Draw the polygon outline
    glLineWidth(2.0f);
    glDrawArrays(GL_LINE_LOOP, 0, vertices.size());

    // Draw points to highlight the vertices
    glPointSize(10.0f);  
    glDrawArrays(GL_POINTS, 0, vertices.size());

    // Unbind and delete VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

}

}
