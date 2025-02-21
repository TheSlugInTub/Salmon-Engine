#pragma once

#include <salmon/components.h>

struct Tilemap
{
    glm::vec2 scale = glm::vec2(1.0f);
    // I swear to god if I have to use any more vectors
    std::vector<glm::mat4> tileTransforms;
    std::vector<float>
        tileTextureIndices; // Index in editorTiles (needs to be float cuz of 
                            // OpenGL, could be an int but i think it's faster
                            // to deal with floats)
    std::vector<unsigned int> editorTiles;     // Tiles you add in the editor
    std::vector<std::string>  editorTilePaths; // Tile paths you add in the editor
};

void TilemapSys();
