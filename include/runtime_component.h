#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <salmon/ecs.h>

// Represents a field in a dynamic component
struct ComponentField {
    std::string name;
    std::string type;
    size_t offset;
    size_t size;
};

// Dynamic component type information
class ComponentType {
public:
    ComponentType(const std::string& name, size_t size) 
        : name(name), totalSize(size) {}

    void addField(const std::string& name, const std::string& type, size_t offset, size_t size) {
        fields.push_back({name, type, offset, size});
    }

    const std::string& GetName() const { return name; }
    size_t GetSize() const { return totalSize; }
    const std::vector<ComponentField>& GetFields() const { return fields; }

private:
    std::string name;
    size_t totalSize;
    std::vector<ComponentField> fields;
};

// Runtime component registry
class RuntimeComponentRegistry {
public:
    static RuntimeComponentRegistry& GetInstance() {
        static RuntimeComponentRegistry instance;
        return instance;
    }

    // Register a new component type
    void RegisterComponent(const std::string& name, const ComponentType& type) {
        componentTypes[name] = type;
        componentIds[name] = componentCounter++;
    }

    // Create a component instance
    void* CreateComponent(Scene& scene, EntityID entity, const std::string& typeName) {
        auto it = componentTypes.find(typeName);
        if (it == componentTypes.end()) return nullptr;

        const ComponentType& type = it->second;
        int componentId = componentIds[typeName];

        // Ensure component pool exists
        if (scene.componentPools.size() <= componentId) {
            scene.componentPools.resize(componentId + 1, nullptr);
        }
        if (scene.componentPools[componentId] == nullptr) {
            scene.componentPools[componentId] = new ComponentPool(type.GetSize());
        }

        // Create component and mark entity
        void* component = scene.componentPools[componentId]->get(GetEntityIndex(entity));
        scene.entities[GetEntityIndex(entity)].mask.set(componentId);
        
        return component;
    }

    // Get component ID
    int GetComponentId(const std::string& typeName) const {
        auto it = componentIds.find(typeName);
        return it != componentIds.end() ? it->second : -1;
    }

private:
    RuntimeComponentRegistry() {}
    std::unordered_map<std::string, ComponentType> componentTypes;
    std::unordered_map<std::string, int> componentIds;
    int componentCounter = MAX_COMPONENTS/2; // Start after compile-time components
};

// Helper function to set field values in dynamic components
template<typename T>
void SetComponentField(void* component, size_t offset, const T& value) {
    *reinterpret_cast<T*>(static_cast<char*>(component) + offset) = value;
}
