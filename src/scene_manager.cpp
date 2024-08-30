#include <scene_manager.h>

namespace SceneManager
{

void SetCurrentScene(Scene& scene)
{
	currentScene = &scene;
}

}