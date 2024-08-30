#pragma once

#include <glm/glm.hpp>
#include <string>

struct Transform
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
};

struct Info
{
	std::string name = "";
	std::string tag = "";
};

struct SpriteRenderer
{
	unsigned int texture;
	std::string texturePath = "";
	glm::vec4 color = glm::vec4(1.0f);
	bool flipped = false;
};