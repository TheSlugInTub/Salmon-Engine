#pragma once

#include <glm/glm.hpp>
#include <salmon/engine.h>
#include <salmon/shader.h>

struct BackgroundSprite
{
    unsigned int texture = 0;
    unsigned int depthTexture = 0;
    std::string texturePath = "";
    std::string depthTexturePath = "";
};

inline unsigned int backgroundVAO, backgroundVBO, backgroundFBO;
inline unsigned int renderPassTexture;
inline Shader backgroundShader;

void GrabRenderPass(int width, int height);
void GrabRenderPassSys();
void BackgroundSpriteSys();
