#include <iostream>
#include <ecs.h>
#include <components.h>
#include <window.h>

Scene scene;

void TransformSys()
{
    for (EntityID ent : SceneView<Transform>(scene))
    {
        auto transform = scene.Get<Transform>(ent);
        std::cout << "Posiiton: " << transform->position.x << std::endl;
    }
}

int main(int argc, char* argv[])
{
    Window window("Slug's Window", 400, 400, false);

    while (!window.ShouldClose())
    {
        scene.UpdateSystems();

        window.Update();
    }

    return 0;
}
