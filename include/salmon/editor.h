#pragma once

#include <imgui/imgui.h>
#include <salmon/engine.h>
#include <salmon/json.hpp>

inline int      selectedEntityIndex = -1;
inline EntityID selectedEntity;

inline bool playing = false;

inline std::string sceneName = "Scene.json";

void DrawHierarchy();
void DrawInspector();
void DrawTray();

void SaveScene(const std::string& filename);
void LoadScene(const std::string& filename);

// Each component type will have its own draw function signature
template<typename T> using ComponentDrawFuncT = void (*)(T*);
template<typename T> using ComponentSaveFuncT = nlohmann::json (*)(T*);
template<typename T> using ComponentLoadFuncT = void (*)(T*, const nlohmann::json&);

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
                return nlohmann::json{};
            });
    }

    template<typename T> void RegisterLoadFunction(ComponentLoadFuncT<T> func, const char* type)
    {
        // Store lambda that captures the typed load function
        loadFunctions[type] = [func](EntityID ent, const nlohmann::json& json)
        {
            if (auto* comp = engineState.scene.Assign<T>(ent))
            {
                func(comp, json); // Call the load function with the component pointer
            }
        };
    }

    template<typename T> void RegisterAddFunction(EntityID* selectedEnt, const char* type)
    {
        // Store lambda that captures the typed load function
        addFunctions.push_back(
            [type, selectedEnt]()
            {
                auto* comp = engineState.scene.Get<T>(*selectedEnt);
                if (ImGui::MenuItem(type) && comp == nullptr)
                {
                    engineState.scene.Assign<T>(*selectedEnt);
                }
            });
    }

    void DrawAll()
    {
        for (const auto& func : drawFunctions) { func(); }
    }

    void AddAll()
    {
        for (const auto& func : addFunctions) { func(); }
    }

    void SaveAll(nlohmann::json& jsonObj)
    {
        for (int i = 0; i < engineState.scene.entities.size(); i++)
        {
            EntityID ent = engineState.scene.entities[i].id;
            if (!IsEntityValid(ent))
                continue;

            nlohmann::json entityData;
            for (const auto& func : saveFunctions)
            {
                std::string    type;
                nlohmann::json compData = func(ent, type);
                compData["type"] = type;
                entityData.push_back(compData);
            }
            jsonObj.push_back(entityData);
        }
    }

    void LoadAll(const nlohmann::json& jsonObj)
    {
        for (const auto& entJson : jsonObj)
        {
            EntityID newEnt = engineState.scene.AddEntity();
            for (const auto& compJson : entJson)
            {
                if (compJson.contains("type"))
                {
                    std::string type = compJson["type"];
                    if (loadFunctions.count(type))
                    {
                        loadFunctions[type](newEnt, compJson);
                    }
                }
            }
        }
    }

  private:
    std::vector<std::function<void()>>                                 drawFunctions;
    std::vector<std::function<nlohmann::json(EntityID, std::string&)>> saveFunctions;
    std::vector<std::function<void()>>                                 addFunctions;
    std::unordered_map<std::string, std::function<void(EntityID, const nlohmann::json&)>>
        loadFunctions;
    ComponentRegistry() = default;
};

// The macro that users will use to register components
#define REGISTER_COMPONENT(ComponentType, DrawFunc, SaveFunc, LoadFunc)                         \
    namespace                                                                                   \
    {                                                                                           \
    struct Register##ComponentType##DrawSaveLoad                                                \
    {                                                                                           \
        Register##ComponentType##DrawSaveLoad()                                                 \
        {                                                                                       \
            ComponentRegistry::Instance().RegisterDrawFunction<ComponentType>(DrawFunc,         \
                                                                              &selectedEntity); \
            ComponentRegistry::Instance().RegisterSaveFunction<ComponentType>(SaveFunc,         \
                                                                              #ComponentType);  \
            ComponentRegistry::Instance().RegisterLoadFunction<ComponentType>(LoadFunc,         \
                                                                              #ComponentType);  \
            ComponentRegistry::Instance().RegisterAddFunction<ComponentType>(&selectedEntity,   \
                                                                             #ComponentType);   \
        }                                                                                       \
    } g_register##ComponentType##DrawSaveLoad;                                                  \
    }
