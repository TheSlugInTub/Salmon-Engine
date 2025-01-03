#pragma once

// This file includes all the other files of the engine
// You should be able to include this file and not have to include anything else

// Macro for clearing the screen
#define ClearScreen()             \
    glClear(GL_COLOR_BUFFER_BIT); \
    glClear(GL_DEPTH_BUFFER_BIT)

// Macro for initializing Jolt physics, this isn't a function because it doesn't register the
// default allocator if it is a function for some reason
#define StartPhysics()                                                                       \
    JPH::RegisterDefaultAllocator();                                                         \
    JPH::Trace = TraceImpl;                                                                  \
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)                             \
    JPH::Factory::sInstance = new JPH::Factory();                                            \
    JPH::RegisterTypes();                                                                    \
    tempAllocator = new JPH::TempAllocatorImpl(200 * 1024 * 1024);                           \
    jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, \
                                             JPH::thread::hardware_concurrency() - 1);       \
    BPLayerInterfaceImpl              broadPhaseLayerInterface;                              \
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter;                         \
    ObjectLayerPairFilterImpl         objectVsObjectLayerFilter;                             \
    physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,   \
                       broadPhaseLayerInterface, objectVsBroadphaseLayerFilter,              \
                       objectVsObjectLayerFilter);

#include <glm/gtc/quaternion.hpp>
#include <salmon/utils.h>
#include <salmon/window.h>
#include <salmon/input.h>
#include <salmon/camera.h>
#include <salmon/shader.h>
#include <salmon/mesh.h>
#include <salmon/model.h>
#include <salmon/ecs.h>
#include <salmon/stb_image.h>
#include <salmon/engine.h>
#include <salmon/components.h>
#include <salmon/renderer.h>
#include <salmon/physics.h>
#include <salmon/imgui_layer.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <salmon/sound_buffer.h>
#include <salmon/sound_device.h>
#include <salmon/sound_source.h>
#include <salmon/particle_system.h>
#include <sm2d/types.h>
#include <sm2d/functions.h>
#include <sm2d/colliders.h>
#include <salmon/clock.h>
#include <salmon/sprite_animation.h>
#include <salmon/editor.h>
