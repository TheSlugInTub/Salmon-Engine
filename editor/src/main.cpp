#include <salmon/salmon.h>
#include <glm/gtx/string_cast.hpp>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 90.0f);

#define GEN_INPUT_FIELD(inputText, target, stmt)                  \
    char##target##Buffer[128];                                    \
    strncpy_s(texBuffer, target.c_str(), sizeof(texBuffer));      \
    if (ImGui::InputText(inputText, texBuffer, sizeof(texBuffer), \
                         ImGuiInputTextFlags_EnterReturnsTrue))   \
    {                                                             \
        stmt                                                      \
    }

namespace Utils
{

template<typename T, typename U, typename V> struct Trio
{
    T key = {};   // Wall Texture
    U value = {}; // res/textures/wall.png
    V tri = {};   // unsigned int texture
};

template<typename T, typename U> struct Pair
{
    T value = {}; // res/textures/wall.png
    U tri = {};   // unsigned int texture
};

template<typename T, typename U, typename V> struct TrioMap
{
public:
    TrioMap(size_t capacity = 1)
    {
        data.reserve(capacity);
    }

    Pair<U, V> Get(const T& key) 
    {
        for (int i = 0; i < data.size(); i++)
        {
            if (data[i].key == key)
            {
                return data[i];
            }
        }
    }

    Pair<U, V>* At(const T& key) 
    {
        for (int i = 0; i < data.size(); i++)
        {
            if (data[i].key == key)
            {
                return &data[i];
            }
        }
    }

    void Insert(Trio<T, U, V> trio)
    {
        data.push_back(trio);
    }

    size_t Size()
    {
        return data.size();
    }

    void Resize(size_t size, Trio<T, U, V> tri = {})
    {
        data.resize(size, tri);
    }

private:
    std::vector<Trio<T, U, V>> data = {};
};

}

// Name, Path, Texture
Utils::TrioMap<std::string, std::string, unsigned int> tiles;

void DrawTilesMenu()
{
    ImGui::Begin("Tiles");

    GEN_INPUT_FIELD("Default Wall Texture", tiles.Get());

    ImGui::End();
}

int main(int argc, char** argv)
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT);
    // glfwSwapInterval(1);

    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");
    unsigned int slugariusTex = Utils::LoadTexture("res/textures/Slugarius.png");

    tiles.Resize(100, {"", "", 0});

    Scene scene;

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init(false, true);

    StartStartSystems();

    ImGuiLayer::Init();

    // Main loop
    // -----------
    while (!window.ShouldClose())
    {
        // Start of frame
        ImGuiLayer::NewFrame();
        UpdateSystems();

        // Main loop logic
        // ---

        DrawTilesMenu();

        // End of frame
        ImGuiLayer::EndFrame();
        window.Update();
    }

    ImGuiLayer::Terminate();

    return 0;
}
