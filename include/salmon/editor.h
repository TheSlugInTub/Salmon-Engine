#pragma once

#include <imgui/imgui.h>
#include <salmon/engine.h>
#include <salmon/json.hpp>

inline int      selectedEntityIndex = -1;
inline EntityID selectedEntity;

void DrawHierarchy();
void DrawInspector();
void DrawTray();

void SaveScene();
void LoadScene();

// Each component type will have its own draw function signature
template<typename T> using ComponentDrawFuncT = void (*)(T*);

// Registry to store component draw functions and their implementations
class ComponentRegistry
{
  public:
    static ComponentRegistry& Instance()
    {
        static ComponentRegistry instance;
        return instance;
    }

    template<typename T>
    void RegisterDrawFunction(ComponentDrawFuncT<T> func, EntityID* selectedEnt)
    {
        // Store lambda that captures the typed draw function
        drawFunctions.push_back(
            [func, selectedEnt]()
            {
                if (auto* comp = engineState.scene.Get<T>(*selectedEnt))
                {
                    func(comp); // Call the draw function with the component pointer
                }
            });
    }

    void DrawAll()
    {
        for (const auto& func : drawFunctions) { func(); }
    }

  private:
    std::vector<std::function<void()>> drawFunctions;
    ComponentRegistry() = default;
};

// The macro that users will use to register components
#define REGISTER_DRAW(ComponentType, DrawFunc)                                                  \
    namespace                                                                                   \
    {                                                                                           \
    struct Register##ComponentType##Draw                                                        \
    {                                                                                           \
        Register##ComponentType##Draw()                                                         \
        {                                                                                       \
            ComponentRegistry::Instance().RegisterDrawFunction<ComponentType>(DrawFunc,         \
                                                                              &selectedEntity); \
        }                                                                                       \
    } g_register##ComponentType##Draw;                                                          \
    }
