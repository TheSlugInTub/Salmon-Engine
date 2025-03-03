#include <salmon/renderer.h>
#include <salmon/engine.h>

void TilemapSys()
{
    for (EntityID ent : SceneView<Tilemap>(engineState.scene))
    {
        auto tilemap = engineState.scene.Get<Tilemap>(ent);

        Renderer::RenderTilemap(*tilemap, engineState.projMat, engineState.camera->GetViewMatrix());
    }
}
