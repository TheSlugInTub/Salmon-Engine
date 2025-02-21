#pragma once

#include <string>
#include <vector>
#include <source_location>
#include <filesystem>
#include <iostream>

#define dbgprint(message)                                            \
    std::source_location location = std::source_location::current(); \
    std::cout << std::filesystem::path(location.file_name()).filename().string() << ": " << message << '\n';

// Utils class
// Has a bunch of useful functions that are used across the engine
namespace Utils
{

// Loads a texture using stb_image and returns an OpenGL texture identifier
unsigned int LoadTexture(const char* path);

// Generates a random floating point value within a range.
// The generated number can be the minimum, but it won't be the maximum
float GenerateRandomNumber(float min, float max);

/*
Returns the index of T in a vector of type T
If a custom type is given, then it must have a == operator
*/
template<typename T> int IndexInVec(std::vector<T>& v, T& K);

} // namespace Utils
