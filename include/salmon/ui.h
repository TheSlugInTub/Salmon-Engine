#pragma once

#include <glm/glm.hpp>
#include <string>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

// Character struct for freetype.
struct Character
{
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    unsigned int Advance;
};

struct Text
{
    std::string text;
    glm::vec2   position;
    glm::vec2   scale;
    float       rotation;
    glm::vec4   color;
    std::string font;
    int         fontPixelSize; // Applies the pixel size to all text with this font
};

struct Button
{
    glm::vec2 position;
    glm::vec2 scale;
    float     rotation;

    glm::vec4    normalColor;
    glm::vec4    hoverColor;
    glm::vec4    pressColor;
    float        transitionSpeed;
    unsigned int texture;

    glm::vec4 color;
    bool      hovered;
};

inline FT_Library                                       ft;
inline std::map<std::string, std::map<char, Character>> fonts;

void TextSys();
void TextStartSys();
void ButtonSys();
void LoadFont(const std::string& fontPath, int pixelSize = 48);
