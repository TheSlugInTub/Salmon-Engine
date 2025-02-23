#include <salmon/engine.h>
#include <salmon/shader.h>
#include <string>

struct BackgroundSprite
{
    unsigned int texture = 0;
    unsigned int depthTexture = 0;
    unsigned int paletteTexture = 0;
    std::string  texturePath = {};
    std::string  depthTexturePath = {};
    std::string  paletteTexturePath = {};

    glm::vec2 dimensions = {};
};

inline unsigned int backgroundFBO;
inline unsigned int renderPassTexture;

inline Shader backgroundShader;

void BackgroundSpriteSys();
void RenderQuadSys();
