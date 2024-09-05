#pragma once

#include <ecs.h>
#include <camera.h>
#include <window.h>

struct EngineState
{
    Scene scene;
    Camera* camera;
    Window* window;

    void SetScene(const Scene& newScene)
    {
        scene = newScene;
    }

    void SetCamera(Camera& newCamera)
    {
        camera = &newCamera;
    }
};  

inline EngineState engineState;
