#include "salmon/input.h"
#include <salmon/ui.h>
#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <salmon/renderer.h>

void TextStartSys()
{
    for (EntityID ent : SceneView<Text>(engineState.scene))
    {
        auto text = engineState.scene.Get<Text>(ent);
        LoadFont(text->font, text->fontPixelSize);
    }
}

void TextSys()
{
    for (EntityID ent : SceneView<Text>(engineState.scene))
    {
        auto text = engineState.scene.Get<Text>(ent);
        Renderer::RenderText(*text, engineState.orthoProjMat);
    }
}

void ButtonSys()
{
    for (EntityID ent : SceneView<Button>(engineState.scene))
    {
        auto button = engineState.scene.Get<Button>(ent);

        Renderer::RenderQuad(button->position, button->scale, button->rotation,
                             engineState.orthoProjMat, button->texture, button->color);

        glm::vec2  objectPos = button->position;
        glm::ivec2 mousePos =
            glm::ivec2(Input::GetMouseInputHorizontal(), Input::GetMouseInputVertical());

        // Invert the Y position of the mouse 
        mousePos.y = engineState.window->height - mousePos.y;

        glm::vec2 minBounds = objectPos - button->scale * 0.5f;
        glm::vec2 maxBounds = objectPos + button->scale * 0.5f;

        // Check if the point is inside the bounds
        if (mousePos.x >= minBounds.x && mousePos.x <= maxBounds.x && mousePos.y >= minBounds.y &&
            mousePos.y <= maxBounds.y)
        {
            button->hovered = true;
            button->color = button->hoverColor;
            if (Input::GetMouseButton(MouseKey::LeftClick))
            {
                button->color = button->pressColor;
            }
            if (Input::GetMouseButtonDown(MouseKey::LeftClick))
            {
                // somethin
            }
        }
        else
        {
            button->hovered = false;
            button->color = button->normalColor;
        }
    }
}

REGISTER_START_SYSTEM(TextStartSys);

void LoadFont(const std::string& fontPath, int pixelSize)
{
    if (fonts.find(fontPath) != fonts.end())
    {
        // Font already loaded
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }
    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    std::map<char, Character> characters;
    for (GLubyte c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {texture,
                               glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                               glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                               static_cast<GLuint>(face->glyph->advance.x)};

        characters.insert(std::pair<char, Character>(c, character));
    }
    fonts.insert(std::pair<std::string, std::map<char, Character>>(fontPath, characters));

    FT_Done_Face(face);
}
