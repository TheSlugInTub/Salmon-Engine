#pragma once

#include <imgui/imgui.h>
#include <salmon/engine.h>
#include <salmon/json.hpp>

inline int      selectedEntityIndex = -1;
inline EntityID selectedEntity;

void DrawHierarchy();
void DrawInspector();
void DrawTray();

void SaveScene(const std::string& filename);
void LoadScene(const std::string& filename);

// Each component type will have its own draw function signature
template<typename T> using ComponentDrawFuncT = void (*)(T*);
template<typename T> using ComponentSaveFuncT = nlohmann::json (*)(T*);

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

    template<typename T> void RegisterSaveFunction(ComponentSaveFuncT<T> func, const char* type)
    {
        // Store lambda that captures the typed save function
        saveFunctions.push_back(
            [func, type](EntityID ent, std::string& compType)
            {
                if (auto* comp = engineState.scene.Get<T>(ent))
                {
                    compType = type;
                    return func(comp); // Call the save function with the component pointer and
                                       // JSON object
                }
            });
    }

    void DrawAll()
    {
        for (const auto& func : drawFunctions) { func(); }
    }

    void SaveAll(nlohmann::json& jsonObj)
    {
        for (int i = 0; i < engineState.scene.entities.size(); i++)
        {
            EntityID       ent = engineState.scene.entities[i].id;
            nlohmann::json entityData;
            nlohmann::json metadata = {{"EntityID", i}};
            entityData.push_back(metadata);
            for (const auto& func : saveFunctions)
            {
                std::string type;
                nlohmann::json compData = func(ent, type);
                compData["type"] = type;
                entityData.push_back(compData);
            }
            jsonObj.push_back(entityData);
        }
    }

  private:
    std::vector<std::function<void()>>                   drawFunctions;
    std::vector<std::function<nlohmann::json(EntityID, std::string&)>> saveFunctions;
    ComponentRegistry() = default;
};

// The macro that users will use to register components
#define REGISTER_COMPONENT(ComponentType, DrawFunc, SaveFunc)                                   \
    namespace                                                                                   \
    {                                                                                           \
    struct Register##ComponentType##DrawSave                                                    \
    {                                                                                           \
        Register##ComponentType##DrawSave()                                                     \
        {                                                                                       \
            ComponentRegistry::Instance().RegisterDrawFunction<ComponentType>(DrawFunc,         \
                                                                              &selectedEntity); \
            ComponentRegistry::Instance().RegisterSaveFunction<ComponentType>(SaveFunc,         \
                                                                              #ComponentType    \
                    );                                                                          \
        }                                                                                       \
    } g_register##ComponentType##DrawSave;                                                      \
    }
