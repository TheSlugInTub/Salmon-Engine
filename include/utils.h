#pragma once

#include <window.h>
#include <camera.h>
#include <string>
#include <vector>
#include <box2d/box2d/box2d.h>

// Utils class
// Has a bunch of useful functions that are used across the engine
namespace Utils
{

// This is probably bad design to have globals but who cares
inline Window* globalWindow;
inline Camera* globalCamera;
inline float deltaTime;
inline b2World* globalWorld;

// Loads a texture using stb_image and returns an OpenGL texture identifier
unsigned int LoadTexture(const char* path);

/* 
Returns the index of T in a vector of type T
If a custom type is given, then it must have a == operator
*/
template<typename T>
int IndexInVec(std::vector<T>& v, T& K);

}