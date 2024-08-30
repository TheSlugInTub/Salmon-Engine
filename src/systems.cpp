#include <scene_manager.h>
#include <ecs.h>
#include <utils.h>
#include <components.h>
#include <renderer.h>

void SpriteRendererSys()
{
    for (EntityID ent : SceneView<Transform, SpriteRenderer>(*SceneManager::currentScene))
    {
        Renderer::Render(ent, Utils::globalCamera->GetViewMatrix(), Utils::globalCamera->GetProjMatrix());
    }
}

REGISTER_SYSTEM(SpriteRendererSys);