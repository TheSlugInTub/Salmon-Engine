#include <iostream>
#include <ecs.h>

// Scene manager for the engine
// Handles the scenes (structs that contain all the entities)

namespace SceneManager
{

// Current scene you're in
inline Scene* currentScene;

// Sets the currentScene to the param
void SetCurrentScene(Scene& scene);

}