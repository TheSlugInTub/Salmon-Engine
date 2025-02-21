#pragma once

#include <salmon/ecs.h>
#include <salmon/camera.h>
#include <salmon/window.h>

// Struct to store the state of the engine
// This is just so I can make the current scene, camera, and window global
// So I can access them from systems to loop over the current scene
struct EngineState
{
    Scene   scene;
    Camera* camera;
    Window* window; // The window class automatically assigns itself to the global engine state on
                    // creation
    float     deltaTime;
    glm::mat4 projMat;
    glm::mat4 orthoProjMat;

    void SetScene(const Scene& newScene) { scene = newScene; }
    void SetCamera(Camera& newCamera)
    {
        camera = &newCamera;
        projMat = camera->GetProjMatrix(window->GetAspectRatio());
        orthoProjMat = glm::ortho(0.0f, (float)window->width, 0.0f, (float)window->height);
    }
};

// Global state of the engine
inline EngineState engineState;
