#include <iostream>
#include <ecs.h>

// Scene manager for the engine
// Handles the scenes (structs that contain all the entities)

namespace SceneManager
{

inline Scene* currentScene;

void SetCurrentScene(Scene& scene);

}