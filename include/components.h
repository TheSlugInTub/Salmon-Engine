#include <ecs.h>
#include <glm/glm.hpp>
#include <model.h>
#include <string>

struct Transform
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

struct MeshRenderer
{
    Model model;
    glm::vec4 color = glm::vec4(1.0f);
    unsigned int texture;
    std::string texturePath = "";
};


