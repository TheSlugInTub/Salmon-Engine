#pragma once

#include <glm/glm.hpp>
#include <string>

struct Transform
{
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

struct Info
{
	std::string name;
	std::string tag;
};