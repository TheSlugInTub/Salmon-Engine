#pragma once

#include <ecs.h>
#include <camera.h>
#include <window.h>

// Struct to store the state of the engine
// This is just so I can make the current scene, camera, and window global
// So I can access them from systems to loop over the current scene
struct EngineState
{
    Scene scene;
    Camera* camera;
    Window* window;
    float deltaTime;

    void SetScene(const Scene& newScene)
    {
        scene = newScene;
    }

    void SetCamera(Camera& newCamera)
    {
        camera = &newCamera;
    }
};  

// Global state of the engine
inline EngineState engineState;
