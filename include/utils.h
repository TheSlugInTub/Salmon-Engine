#pragma once

#include <window.h>
#include <camera.h>
#include <string>

namespace Utils
{

inline Window* globalWindow;
inline Camera* globalCamera;

unsigned int LoadTexture(const char* path);

}