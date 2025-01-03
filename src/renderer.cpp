#include "salmon/shader.h"
#include <salmon/renderer.h>
#include <salmon/model.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <salmon/engine.h>
#include <salmon/components.h>
#include <salmon/utils.h>
#include <salmon/stb_image.h>
#include <salmon/particle_system.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>
#include <salmon/clock.h>

namespace Renderer
{

// This function is run at the very start of the program
void Init(bool depth, bool ui)
{
    defaultShader = Shader("shaders/3d_vertex.shad", "shaders/3d_fragment.shad");
    lineShader = Shader("shaders/line_vertex.shad", "shaders/line_fragment.shad");
    depthShader = Shader("shaders/shadow_vertex.shad", "shaders/shadow_fragment.shad",
                         "shaders/shadow_geometry.shad");
    twoShader = Shader("shaders/2d_vertex.shad", "shaders/2d_fragment.shad");
    parShader = Shader("shaders/particle_vertex.shad", "shaders/particle_fragment.shad");
    textShader = Shader("shaders/text_vertex.shad", "shaders/text_fragment.shad");
    stbi_set_flip_vertically_on_load(true);

    /*
    These are vertices for rendering a triangle and the indices make it a quad,
    since those are all we need for a 2D renderer.
    */
    float vertices[] = {
        // positions         // texture coords
        0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // top right
        -0.5f, 0.5f,  0.0f, 0.0f, 1.0f, // top left
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        0.5f,  -0.5f, 0.0f, 1.0f, 0.0f  // bottom right
    };

    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
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

    particleMatrices.reserve(MAX_PARTICLES);

    glGenBuffers(1, &instancedVBO);

    // Reserve space for instance transformation matrices
    glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    // Enable instanced attributes (mat4 takes 4 vec4 attributes)
    glBindVertexArray(VAO);
    for (int i = 0; i < 4; i++)
    {
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                              (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(2 + i);
        glVertexAttribDivisor(2 + i, 1); // Instance divisor for instancing
    }

    glGenBuffers(1, &instancedColorVBO);

    // Reserve space for instance colors
    glBindBuffer(GL_ARRAY_BUFFER, instancedColorVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::vec4), nullptr, GL_DYNAMIC_DRAW);

    // Enable the instance color attribute
    glBindVertexArray(VAO);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
    glEnableVertexAttribArray(6);
    glVertexAttribDivisor(6, 1); // Color updates per instance
    glBindVertexArray(0);

    glBindVertexArray(0);

    // Font rendering
    if (ui)
    {
        if (FT_Init_FreeType(&ft))
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        }
    }
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Enable the DEPTH_TEST, basically just so faces don't draw on top of eachother in weird ways
    if (depth)
    {
        glEnable(GL_DEPTH_TEST);
    }
    // Culls inside faces to save on performance
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // Enables anti-aliasing
    glEnable(GL_MULTISAMPLE);
    // Enables transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// This function is run for every model in the scene
void RenderModel(EntityID ent, const glm::mat4& projection, const glm::mat4& view)
{
    // Gets the components of the entity
    auto trans = engineState.scene.Get<Transform>(ent);
    auto model = engineState.scene.Get<MeshRenderer>(ent);

    glm::mat4 transform;

    if (trans->useMatrix == false)
    {
        transform = MakeModelTransform(trans);
    }
    else
    {
        transform = trans->modelMat;
    }

    // Activate the shader program
    defaultShader.use();

    // Setting all the uniforms.
    defaultShader.setMat4("model", transform);
    defaultShader.setMat4("view", view);
    defaultShader.setMat4("projection", projection);
    defaultShader.setTexture2D("texture_diffuse", model->texture, 0);

    for (int i = 0; i < lights.size(); ++i)
    {
        if (!lights[i].castShadows)
        {
            break;
        }
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
        defaultShader.setBool(baseName + ".castShadows", lights[i].castShadows);
    }

    auto anim = engineState.scene.Get<Animator>(ent);

    defaultShader.setBool("useAnim", false);

    if (anim != nullptr)
    {
        auto transforms = anim->boneMatrices;
        for (int i = 0; i < transforms.size(); ++i)
        {
            defaultShader.setBool("useAnim", true);
            defaultShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }
    }

    // Draws the model
    model->model.Draw(defaultShader);

    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::mat4 MakeModelTransform(Transform* trans)
{
    glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform
    transform = glm::translate(transform, trans->position);
    transform = glm::rotate(transform, trans->rotation.x,
                            glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation around X-axis
    transform = glm::rotate(transform, trans->rotation.y,
                            glm::vec3(0.0f, 1.0f, 0.0f)); // Rotation around Y-axis
    transform = glm::rotate(transform, trans->rotation.z,
                            glm::vec3(0.0f, 0.0f, 1.0f)); // Rotation around Z-axis
    transform = glm::scale(transform, trans->scale);

    return transform;
}

void RenderLine(const std::vector<glm::vec3>& points, const glm::mat4& projection,
                const glm::mat4& view, const glm::vec4& color)
{
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
    lineShader.setVec4("color", color);

    // Draw line
    glLineWidth(3.0f);
    glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)points.size());

    // Draw points
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, (GLsizei)points.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &lVBO);
    glDeleteVertexArrays(1, &lVAO);
}

void RenderSprite(EntityID ent, const glm::mat4& projection, const glm::mat4& view)
{
    auto sprite = engineState.scene.Get<SpriteRenderer>(ent);
    auto trans = engineState.scene.Get<Transform>(ent);

    if (sprite->color.w == 0)
    {
        // If the alpha of the object is zero, then don't bother with rendering it.
        return;
    }

    twoShader.use();
    twoShader.setTexture2D("texture1", sprite->texture, 0);

    glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform.
    transform = glm::translate(transform,
                               glm::vec3(trans->position.x, trans->position.y, trans->position.z));

    if (!sprite->billboard)
    {
        transform = glm::rotate(transform, trans->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else
    {
        // Cancel out view rotation for billboarding
        glm::mat4 rotationCancel = glm::transpose(glm::mat3(view));
        transform = transform * glm::mat4(rotationCancel);
    }

    transform = glm::scale(transform, glm::vec3(trans->scale.x, trans->scale.y, 1.0f));

    // Setting all the uniforms.
    twoShader.setMat4("model", transform);
    twoShader.setMat4("view", view);
    twoShader.setMat4("projection", projection);
    twoShader.setVec4("ourColor", sprite->color);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void RenderParticleSystem(const ParticleSystem& par, const glm::mat4& projection,
                          const glm::mat4& view)
{
    if (par.particles.size() == 0)
        return;

    glDisable(GL_DEPTH_TEST);

    particleMatrices.clear();
    for (int i = 0; i < par.particles.size(); ++i)
    {
        particleMatrices.push_back(glm::mat4(1.0f));

        // Apply position to he matrix
        particleMatrices[i] = glm::translate(
            particleMatrices[i], glm::vec3(par.particles[i].position.x, par.particles[i].position.y,
                                           par.particles[i].position.z));
        // Cancel out rotation for the billboarded alignment
        glm::mat4 rotationCancel = glm::transpose(glm::mat3(view));
        particleMatrices[i] = particleMatrices[i] * glm::mat4(rotationCancel);

        // Apply 2D rotation (around Z-axis) for custom billboard rotation
        float     angle = par.particles[i].rotation.z; // The rotation angle in radians
        glm::mat4 rotation2D = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
        particleMatrices[i] = particleMatrices[i] * rotation2D;

        // Scale the matrix
        particleMatrices[i] = glm::scale(
            particleMatrices[i], glm::vec3(par.particles[i].size.x, par.particles[i].size.y, 1.0f));
    }

    // Update instance transformation data
    glBindBuffer(GL_ARRAY_BUFFER, instancedVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, par.particles.size() * sizeof(glm::mat4),
                    particleMatrices.data());

    std::vector<glm::vec4> colors(par.particles.size());
    for (size_t i = 0; i < par.particles.size(); ++i) { colors[i] = par.particles[i].color; }

    glBindBuffer(GL_ARRAY_BUFFER, instancedColorVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, par.particles.size() * sizeof(glm::vec4), colors.data());

    parShader.use();
    parShader.setTexture2D("texture1", par.texture, 0);

    // Setting all the uniforms.
    parShader.setMat4("view", view);
    parShader.setMat4("projection", projection);
    parShader.setVec4("ourColor", par.startingColor);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, par.particles.size());

    glEnable(GL_DEPTH_TEST);
}

void RenderText(const Text& item, const glm::mat4& projection)
{
    textShader.use();
    glBindVertexArray(textVAO);

    const std::string& text = item.text;
    const std::string& fontPath = item.font;
    float              x = item.position.x;
    float              y = item.position.y;
    glm::vec2          scale = item.scale; // Scaling factors for x and y

    textShader.setVec4("textColor", item.color);
    textShader.setMat4("projection", projection);

    // Calculate the total width of the text
    float totalWidth = 0.0f;
    for (auto c = text.begin(); c != text.end(); ++c)
    {
        Character ch = fonts[fontPath][*c];
        totalWidth += (ch.Advance >> 6) * scale.x; // Advance is in 1/64 pixels
    }

    // Adjust the starting x position to center the text
    float startX = x - totalWidth / 2.0f;

    for (auto c = text.begin(); c != text.end(); ++c)
    {
        Character ch = fonts[fontPath][*c];

        float xpos = startX + ch.Bearing.x * scale.x;          // Apply x scaling
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale.y; // Apply y scaling

        float w = ch.Size.x * scale.x; // Apply x scaling
        float h = ch.Size.y * scale.y; // Apply y scaling
        float vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                                {xpos + w, ypos, 1.0f, 1.0f},

                                {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                                {xpos + w, ypos + h, 1.0f, 0.0f}};
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        startX += (ch.Advance >> 6) * scale.x; // Move to the next character position
    }
    glBindVertexArray(0);
}

void RenderQuad(glm::vec2 position, glm::vec2 scale, float rotation, const glm::mat4& projection,
                unsigned int texture, const glm::vec4& color)
{
    if (color.w == 0)
    {
        // If the alpha of the object is zero, then don't bother with rendering it.
        return;
    }

    twoShader.use();
    twoShader.setTexture2D("texture1", texture, 0);

    glm::mat4 transform = glm::mat4(1.0f);

    // Matrix multiplication to calculate the transform.
    transform = glm::translate(transform, glm::vec3(position.x, position.y, 0.0));

    transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));

    transform = glm::scale(transform, glm::vec3(scale.x, scale.y, 1.0f));

    glm::mat4 view = glm::mat4(1.0f);

    // Setting all the uniforms.
    twoShader.setMat4("model", transform);
    twoShader.setMat4("view", view);
    twoShader.setMat4("projection", projection);
    twoShader.setVec4("ourColor", color);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

} // namespace Renderer
