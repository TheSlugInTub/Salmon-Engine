#include <salmon/engine.h>
#include <salmon/shader.h>
#include <string>

struct BackgroundSprite
{
    unsigned int texture;
    unsigned int depthTexture;
    std::string  texturePath;
    std::string  depthTexturePath;
};

inline unsigned int backgroundFBO;
inline unsigned int renderPassTexture;

inline Shader backgroundShader;

void BackgroundSpriteSys();
void RenderQuadSys();
