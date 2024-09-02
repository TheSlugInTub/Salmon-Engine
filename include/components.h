#pragma once

#include <glm/glm.hpp>
#include <string>
#include <box2d/box2d/box2d.h>

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
	std::string texturePath = "res/textures/DefaultTexture.png";
	glm::vec4 color = glm::vec4(1.0f);
	bool flipped = false;
};

// Rigidbody, adds physics to an entity's transform
struct RigidBody2D
{
	glm::vec2 bodyScale = glm::vec2(0.5f);
	b2Body* body = nullptr;
	b2BodyType type = b2_staticBody;
	// First frame of rigidbody's existence (used for making a b2Body at first frame)
	bool start = true;
};