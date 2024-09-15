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

// This function is run at the very start of the program
void Init()
{
    Shader newShader("shaders/vertex.shad", "shaders/fragment.shad");
    defaultShader = newShader;
    Shader newLineShader("shaders/linevertex.shad", "shaders/linefragment.shad");
    lineShader = newLineShader;
    Shader newDepthShader("shaders/shadowvertex.shad", "shaders/shadowfragment.shad", "shaders/shadowgeometry.shad");
    depthShader = newDepthShader;

    stbi_set_flip_vertically_on_load(true);

    // Enable the DEPTH_TEST, basically just so faces don't draw on top of eachother in weird ways
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

// This function is run for every model in the scene
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view)
{
    // Gets the components of the entity
    auto trans = engineState.scene.Get<Transform>(ent);
    auto model = engineState.scene.Get<MeshRenderer>(ent);

    glm::mat4 transform = MakeModelTransform(trans);

    // Activate the shader program
    defaultShader.use();
	
    // Setting all the uniforms.
    defaultShader.setMat4("model", transform);
    defaultShader.setMat4("view", view);
    defaultShader.setMat4("projection", projection);
    defaultShader.setFloat("farPlane", 25.0f);
    defaultShader.setTexture2D("texture_diffuse", model->texture, 0);

    for (int i = 0; i < lights.size(); ++i)
    {
        // Bind each light's shadow map to a different texture unit
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_CUBE_MAP, lights[i].depthCubemap);
    
        // Set the corresponding sampler in the shader
        Renderer::defaultShader.setInt("depthMaps[" + std::to_string(i) + "]", i);
    }

    for (int i = 0; i < lights.size(); ++i)
    {
        std::string baseName = "lights[" + std::to_string(i) + "]";
        defaultShader.setVec3(baseName + ".pos", lights[i].position);
        defaultShader.setFloat(baseName + ".radius", lights[i].radius);
        defaultShader.setVec4(baseName + ".color", lights[i].color);
        defaultShader.setFloat(baseName + ".intensity", lights[i].intensity);
        defaultShader.setBool(baseName + ".on", true);
    }

    // Draws the model
    model->model.Draw(defaultShader);
}

glm::mat4 MakeModelTransform(Transform* trans)
{
    glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform.
    transform = glm::translate(transform, trans->position);
    transform = glm::rotate(transform, trans->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation around X-axis
    transform = glm::rotate(transform, trans->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation around Y-axis
    transform = glm::rotate(transform, trans->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // Rotation around Z-axis
    transform = glm::scale(transform, trans->scale);

    return transform;
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
