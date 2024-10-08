#pragma once

// This file includes all the other files of the engine
// You should be able to include this file and not have to include anything else

// Macro for clearing the screen
#define ClearScreen() glClear(GL_COLOR_BUFFER_BIT); glClear(GL_DEPTH_BUFFER_BIT)

// Macro for initializing Jolt physics, this isn't a function because it doesn't register the default allocator if it is a function for some reason
#define StartPhysics() \
    RegisterDefaultAllocator(); \
    Trace = TraceImpl; \
    JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;) \
    Factory::sInstance = new Factory(); \
    RegisterTypes(); \
    tempAllocator = new TempAllocatorImpl(200 * 1024 * 1024); \
    jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1); \
    BPLayerInterfaceImpl broadPhaseLayerInterface; \
    ObjectVsBroadPhaseLayerFilterImpl objectVsBroadphaseLayerFilter; \
    ObjectLayerPairFilterImpl objectVsObjectLayerFilter; \
    physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broadPhaseLayerInterface, objectVsBroadphaseLayerFilter, objectVsObjectLayerFilter);

#include <glm/gtc/quaternion.hpp>
#include <utils.h>
#include <window.h>
#include <input.h>
#include <camera.h>
#include <shader.h>
#include <mesh.h>
#include <model.h>
#include <ecs.h>
#include <stb_image.h>
#include <engine.h>
#include <components.h>
#include <renderer.h>
#include <physics.h>
#include <imgui_layer.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <sound_buffer.h>
#include <sound_device.h>
#include <sound_source.h>
