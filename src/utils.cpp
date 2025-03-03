#include <salmon/utils.h>
#include <salmon/stb_image.h>
#include <glad/glad.h>

namespace Utils
{

unsigned int LoadTexture(const char* path, bool flip, bool glFloat)
{
    stbi_set_flip_vertically_on_load(flip);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int            width, height, nrComponents;
    unsigned char* data =
        stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                     format, glFloat ? GL_FLOAT : GL_UNSIGNED_BYTE,
                     data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                          : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                          : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_NEAREST);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path
                  << '\n';
        stbi_image_free(data);
        return LoadTexture("res/textures/MissingTexture.png", flip);
    }

    return textureID;
}

unsigned int LoadTexture(const char* path, bool flip,
                         glm::vec2& dimensions)
{
    stbi_set_flip_vertically_on_load(flip);
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int            width, height, nrComponents;
    unsigned char* data =
        stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                        format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                          : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                        format == GL_RGBA ? GL_CLAMP_TO_EDGE
                                          : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                        GL_NEAREST);

        dimensions = glm::vec2(width, height);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path
                  << '\n';
        stbi_image_free(data);
        return LoadTexture("res/textures/MissingTexture.png", flip);
    }

    return textureID;
}

template<typename T> int IndexInVec(std::vector<T>& v, T& K)
{
    auto it = std::find(v.begin(), v.end(), K);
    if (it != v.end())
    {
        return std::distance(v.begin(), it);
    }
    return -1;
}

float GenerateRandomNumber(float min, float max)
{
    std::random_device rd;
    std::mt19937       gen(rd()); // Mersenne Twister engine
    std::uniform_real_distribution<float> dist(
        min, max); // Distribution in range [min, max]

    return dist(gen);
}

glm::mat4 Make2DTransform(const glm::vec3& position, float rotation,
                          const glm::vec2& scale)
{

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform =
        glm::scale(transform, glm::vec3(scale.x, scale.y, 1.0f));
    transform =
        glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    return transform;
}

glm::vec3 GetPositionOfMat4(const glm::mat4& mat)
{
    return glm::vec3(mat[3]);
}

} // namespace Utils
