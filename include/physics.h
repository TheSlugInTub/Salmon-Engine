#pragma once

// This file contains all of the stuff related to Jolt Physics
// Contains stuff like structs and initialization of various things because jolt needs them

#include "input.h"
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/TriangleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <renderer.h>
#include <engine.h>
#include <functional>

#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#else
	// Hack to still compile DebugRenderer inside the test framework when Jolt is compiled without
	#define JPH_DEBUG_RENDERER
	// Make sure the debug renderer symbols don't get imported or exported
	#define JPH_DEBUG_RENDERER_EXPORT
	#include <Jolt/Renderer/DebugRenderer.h>
	#undef JPH_DEBUG_RENDERER
	#undef JPH_DEBUG_RENDERER_EXPORT
#endif

#ifndef JPH_ENABLE_ASSERTS
#define JPH_ENABLE_ASSERTS
#endif

#include <Jolt/Core/Mutex.h>
#include <Jolt/Core/UnorderedMap.h>

// STL includes
#include <iostream>
#include <cstdarg>
#include <thread>

// Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
JPH_SUPPRESS_WARNINGS

// If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.
using namespace JPH::literals;

namespace Layers
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
    std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
{
	// Print to the TTY
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;

	// Breakpoint
	return true;
};

#endif // JPH_ENABLE_ASSERTS

inline unsigned int cMaxBodies = 10240;
inline unsigned int cNumBodyMutexes = 0; // Autodetect
inline unsigned int cMaxBodyPairs = 65536;
inline unsigned int cMaxContactConstraints = 20480;

inline JPH::PhysicsSystem physicsSystem;
inline JPH::TempAllocatorImpl* tempAllocator;
inline JPH::JobSystemThreadPool* jobSystem;

inline void StepPhysics(float deltaTime)
{
    physicsSystem.Update(deltaTime, 1, tempAllocator, jobSystem);
}

inline void DestroyPhysics()
{
    delete tempAllocator;
    delete jobSystem;

    JPH::UnregisterTypes();

    // Destroy the factory
    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

struct LineSeg
{
    glm::vec3 inFrom;
    glm::vec3 inTo;
};

inline JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();

class MyDebugRenderer final : public JPH::DebugRenderer {
public:
	JPH_OVERRIDE_NEW_DELETE

    MyDebugRenderer()
    {}

    virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override 
    {
        LineSeg line(glm::vec3(inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()), glm::vec3(inTo.GetX(), inTo.GetY(), inTo.GetZ()));

		float aspectRatio = engineState.window->GetAspectRatio();
    	Renderer::RenderLine(line.inFrom, line.inTo, engineState.camera->GetProjMatrix(aspectRatio), engineState.camera->GetViewMatrix());
    }

	inline void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow = ECastShadow::Off) override
    {
        std::cout << "Triangle has been drawn!";
    }

	inline virtual Batch CreateTriangleBatch(const Triangle *inTriangles, int inTriangleCount)
    {
        //fuck off
        return Batch{};
    }

    inline virtual Batch CreateTriangleBatch(const Vertex *inVertices, int inVertexCount, const JPH::uint32 *inIndices, int inIndexCount)
    {
       	return Batch{};
    }

    inline virtual void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox &inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef &inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid)
    {
        std::cout << "Geometry has been drawn!";
    }

    inline virtual void DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view &inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f)
    {
   		//fuck off
    }
};

struct CollisionEventData
{
    const JPH::Body* id;
    std::function<void(const JPH::Body* id1, const JPH::Body* id2)> call;
    bool collide;
};

inline std::vector<CollisionEventData> registeredCollisions;

inline void AddCollisionEnterEvent(JPH::Body* id, std::function<void(const JPH::Body* id1, const JPH::Body* id2)> call)
{
    CollisionEventData data(id, call, true);
    registeredCollisions.push_back(data);
}

class MyContactListener : public JPH::ContactListener {
public:
    // Called when two bodies start colliding
    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg baseOffset, const JPH::CollideShapeResult& collisionResult) override 
    {
        // Decide whether the collision should be processed
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    // Called when two bodies are colliding, called for each contact point
    virtual void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override 
    {
        std::cout << "Contact added\n";

        for (const auto& data : registeredCollisions)
        {
            if (body1.GetID() == data.id->GetID() || body2.GetID() == data.id->GetID())
            {
                data.call(&body1, &body2);
                std::cout << "Callback called\n";
            }
        }
    }

    // Called when two bodies stop colliding
    virtual void OnContactRemoved(const JPH::SubShapeIDPair& subShapePair) override 
    {
        //for (auto data : registeredDecollisions)
        //{
        //    data.call(subShapePair.GetBody1ID(), subShapePair.GetBody2ID());
        //}
    }
};
