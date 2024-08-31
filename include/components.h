#pragma once

#include <glm/glm.hpp>
#include <string>

// Useful components for the game engine

// Transform component, used on all entities, represents a point in space
struct Transform
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
};

// Info component, used on all entities, some basic information to display entities
struct Info
{
	std::string name = "";
	std::string tag = "";
};

// Sprite component, its system renders a quad, with a texture
struct SpriteRenderer
{
	unsigned int texture;
	std::string texturePath = "";
	glm::vec4 color = glm::vec4(1.0f);
	bool flipped = false;
};