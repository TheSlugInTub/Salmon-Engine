#pragma once

#include <string>
#include <vector>

// Utils class
// Has a bunch of useful functions that are used across the engine
namespace Utils
{

// Loads a texture using stb_image and returns an OpenGL texture identifier
unsigned int LoadTexture(const char* path);

/* 
Returns the index of T in a vector of type T
If a custom type is given, then it must have a == operator
*/
template<typename T>
int IndexInVec(std::vector<T>& v, T& K);

}
