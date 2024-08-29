#pragma once

#include <bitset>
#include <vector>
#include <functional>

int componentCounter = 0;

typedef unsigned long long EntityID;
const int MAX_COMPONENTS = 32;
typedef std::bitset<MAX_COMPONENTS> ComponentMask;

typedef unsigned int EntityIndex;
typedef unsigned int EntityVersion;

inline EntityID CreateEntityId(EntityIndex index, EntityVersion version)
{
  	// Shift the index up 32, and put the version in the bottom
  	return ((EntityID)index << 32) | ((EntityID)version);
}
inline EntityIndex GetEntityIndex(EntityID id)
{
  	// Shift down 32 so we lose the version and get our index
  	return id >> 32;
}
inline EntityVersion GetEntityVersion(EntityID id)
{
  	// Cast to a 32 bit int to get our version number (loosing the top 32 bits)
  	return (EntityVersion)id;
}
inline bool IsEntityValid(EntityID id)
{
  	// Check if the index is our invalid index
  	return (id >> 32) != EntityIndex(-1);
}
inline bool ValidIndex()
{
  	return
    	// It's a valid entity ID
    	IsEntityValid(pScene->entities[index].id) &&
    	// It has the correct component mask
    	(all || mask == (mask & pScene->entities[index].mask));
}

#define INVALID_ENTITY CreateEntityId(EntityIndex(-1), 0)

// Gets the id of a component
template<class T>
int GetId()
{
	static int componentId = componentCounter++;
	return componentId;
}

struct Scene
{
	// Struct to store all the information needed for an entity
  	struct EntityDesc
  	{
    	EntityID id;
    	ComponentMask mask;
  	};

  	// Creates an entity in the scene
	EntityID AddEntity()
	{
	  	if (!freeEntities.empty())
	  	{
	    	EntityIndex newIndex = freeEntities.back();
	    	freeEntities.pop_back();
	    	EntityID newID = CreateEntityId(newIndex, GetEntityVersion(entities[newIndex].id));
	    	entities[newIndex].id = newID;
	    	return entities[newIndex].id;
	  	}
	  	entities.push_back({ CreateEntityId(EntityIndex(entities.size()), 0), ComponentMask() });
	  	return entities.back().id;
	}

	// Assigns a component to an entity id
	template<typename T>
	T* Assign(EntityID id)
	{
	    int componentId = GetId<T>();
  
	    if (componentPools.size() <= componentId) // Not enough component pool
	    {
	      	componentPools.resize(componentId + 1, nullptr);
	    }
	    if (componentPools[componentId] == nullptr) // New component, make a new pool
	    {
	      	componentPools[componentId] = new ComponentPool(sizeof(T));
	    }
  
	    // Looks up the component in the pool, and initializes it with placement new
	    T* pComponent = new (componentPools[componentId]->get(GetEntityIndex(id))) T();
  
	    // Set the bit for this component to true and return the created component
	    entities[GetEntityIndex(id)].mask.set(componentId);
	    return pComponent;
	}

	// Retrieves a given component from an entity id
	template<typename T>
	T* Get(EntityID id)
	{
	  	int componentId = GetId<T>();
	  	if (!entities[GetEntityIndex(id)].mask.test(componentId))
	    	return nullptr;

	  	T* pComponent = static_cast<T*>(componentPools[componentId]->get(GetEntityIndex(id)));
	  	return pComponent;
	}

	template<typename T>
	void Remove(EntityID id)
	{
	  	// ensures you're not accessing an entity that has been deleted
	  	if (entities[GetEntityIndex(id)].id != id) 
	    	return;

	  	int componentId = GetId<T>();
	  	entities[GetEntityIndex(id)].mask.reset(componentId);
	}

	void DestroyEntity(EntityID id)
	{
	  	EntityID newID = CreateEntityId(EntityIndex(-1), GetEntityVersion(id) + 1);
	  	entities[GetEntityIndex(id)].id = newID;
	  	entities[GetEntityIndex(id)].mask.reset(); 
	  	freeEntities.push_back(GetEntityIndex(id));
	}

  	std::vector<EntityDesc> entities;
  	std::vector<EntityIndex> freeEntities;
  	std::vector<ComponentPool> componentPools;
  	std::vector<std::function<void>> systems;
};

// Memory pool
struct ComponentPool
{
	ComponentPool(size_t elementsize)
	{
	  	// We'll allocate enough memory to hold MAX_ENTITIES, each with element size
	  	elementSize = elementsize;
	  	pData = new char[elementSize * MAX_ENTITIES];
	}

	~ComponentPool()
	{
	  	delete[] pData;
	}

	inline void* get(size_t index)
	{
	  	// looking up the component at the desired index
	  	return pData + index * elementSize;
	}

	char* pData{ nullptr };
	size_t elementSize{ 0 };
};

template<typename... ComponentTypes>
struct SceneView
{
  	SceneView(Scene& scene) : pScene(&scene) 
  	{
    	if (sizeof...(ComponentTypes) == 0)
    	{
      		all = true;
    	}
    	else
    	{
      		// Unpack the template parameters into an initializer list
      		int componentIds[] = { 0, GetId<ComponentTypes>() ... };

      		for (int i = 1; i < (sizeof...(ComponentTypes) + 1); i++)
        		componentMask.set(componentIds[i]);
    	}
  	}

  	struct Iterator
  	{
  		Iterator(Scene* pScene, EntityIndex index, ComponentMask mask, bool all) 
    		: pScene(pScene), index(index), mask(mask), all(all) {}
    	
    	EntityID operator*() const
    	{
      		return pScene->entities[index].id;
    	}
    
	    bool operator==(const Iterator& other) const
	    {
	      	return index == other.index || index == pScene->entities.size();
	    }

	    bool operator!=(const Iterator& other) const
	    {
	     	return index != other.index && index != pScene->entities.size();
	    }

	    Iterator& operator++()
	    {
		  	do
		  	{
		    	index++;
		  	} 
		  	while (index < pScene->entities.size() && !ValidIndex());
		  	return *this;
	    }

	  	EntityIndex index;
	  	Scene* pScene;
	  	ComponentMask mask;
	  	bool all{ false };
  	};

  	const Iterator begin() const
  	{
	  	int firstIndex = 0;
	  	while (firstIndex < pScene->entities.size() &&
	    	(componentMask != (componentMask & pScene->entities[firstIndex].mask) 
	      	|| !IsEntityValid(pScene->entities[firstIndex].id))) 
	  	{
	    	firstIndex++;
	  	}
	  	return Iterator(pScene, firstIndex, componentMask, all);
  	}

  	const Iterator end() const
  	{
		return Iterator(pScene, EntityIndex(pScene->entities.size()), componentMask, all);
  	}

  	Scene* pScene{ nullptr };
  	ComponentMask componentMask;
  	bool all{ false };
};