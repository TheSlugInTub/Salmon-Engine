#include <iostream>
#include <ecs.h>
#include <components.h>

int main(int argc, char* argv[])
{
    Scene scene;

    EntityID newEnt = scene.AddEntity();
    scene.Assign<Transform>(newEnt);
}
