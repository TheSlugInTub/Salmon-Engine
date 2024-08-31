#include <renderer.h>
#include <glad/glad.h>
#include <utils.h>
#include <scene_manager.h>
#include <components.h>

namespace Renderer
{

void Init()
{
    Shader newShader("shaders/vertex.shad", "shaders/fragment.shad");
    defaultShader = newShader;

    /*
    Vertices for a cube, including positions and texture coordinates.
    A cube has six faces, each defined by two triangles (6 vertices per face).
    */
    float vertices[] = {
        // positions          // texture coords
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top right
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top left

        // Back face
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom right
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom left
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top left
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top right

        // Left face
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top right
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, // top left
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom left
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, // bottom right

        // Right face
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // top left
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, // top right
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // bottom right
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, // bottom left

        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, // top left
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, // top right
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // bottom right
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // bottom left

        // Bottom face
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // bottom right
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom left
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // top left
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f  // top right
    };

    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        // Back face
        4, 5, 6,
        6, 7, 4,
        // Left face
        8, 9, 10,
        10, 11, 8,
        // Right face
        12, 13, 14,
        14, 15, 12,
        // Top face
        16, 17, 18,
        18, 19, 16,
        // Bottom face
        20, 21, 22,
        22, 23, 20
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_DEPTH_TEST);
}

void Render(EntityID ent, const glm::mat4& view, const glm::mat4& projection)
{
	auto sprite = SceneManager::currentScene->Get<SpriteRenderer>(ent);
	auto trans = SceneManager::currentScene->Get<Transform>(ent);

	defaultShader.use();
	defaultShader.setTexture2D("texture1", sprite->texture, 0);

	glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform.
	transform = glm::translate(transform, trans->position);

	transform = glm::rotate(transform, trans->rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	transform = glm::rotate(transform, trans->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	transform = glm::rotate(transform, trans->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	
	transform = glm::scale(transform, trans->scale);

    // Setting all the uniforms.
	defaultShader.setMat4("transform", transform);
	defaultShader.setMat4("view", view);
	defaultShader.setMat4("projection", projection);

	defaultShader.setVec4("ourColor", sprite->color);

    // This changes the vertices of the quad if the object is flipped.
    // float vertices[] = {
    //     // positions     // texture coords
    //     0.5f,  0.5f, 0.0f, sprite->flipped ? 0.0f : 1.0f, 1.0f,   // top right (flipped if true)
    //     0.5f, -0.5f, 0.0f, sprite->flipped ? 0.0f : 1.0f, 0.0f,   // bottom right (flipped if true)
    //    -0.5f, -0.5f, 0.0f, sprite->flipped ? 1.0f : 0.0f, 0.0f,   // bottom left (flipped if true)
    //    -0.5f,  0.5f, 0.0f, sprite->flipped ? 1.0f : 0.0f, 1.0f    // top left (flipped if true)
    // };

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

}