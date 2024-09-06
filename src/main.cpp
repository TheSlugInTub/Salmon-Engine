#include <salmon.h>
#include <iostream>
#include <string>

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int cMaxBodies = 10240;
unsigned int cNumBodyMutexes = 0; // Autodetect
unsigned int cMaxBodyPairs = 65536;
unsigned int cMaxContactConstraints = 20480;

using namespace JPH;

int main()
{
    Window window("Prism", SCR_WIDTH, SCR_HEIGHT, false);
    glfwSetInputMode(window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    RegisterDefaultAllocator();
	
	// Install trace and assert callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

    Factory::sInstance = new Factory();
	RegisterTypes();

	TempAllocatorImpl temp_allocator(200 * 1024 * 1024);
	JobSystemThreadPool job_system(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	BPLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	PhysicsSystem physics_system;
	physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);


    Model ourModel("res/models/Box.obj");
    unsigned int tex = Utils::LoadTexture("res/textures/Slugarius.png");
    unsigned int groundTex = Utils::LoadTexture("res/textures/background.png");

    Scene scene;
    EntityID ent = scene.AddEntity();
    scene.AssignParam<Transform>(ent, glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    scene.AssignParam<MeshRenderer>(ent, ourModel, glm::vec4(1.0f), tex);

    EntityID ground = scene.AddEntity();
    scene.AssignParam<Transform>(ground, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f));
    scene.AssignParam<MeshRenderer>(ground, ourModel, glm::vec4(1.0f), groundTex);

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	BodyInterface &body_interface = physics_system.GetBodyInterface();

	// Next we can create a rigid body to serve as the floor, we make a large box
	// Create the settings for the collision volume (the shape).
	// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
	BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.5f, 100.0f));
	floor_shape_settings.SetEmbedded(); // A ref counted object on the stack (base class RefTarget) should be marked as such to prevent it from being freed when its reference count goes to 0.

	// Create the shape
	ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

	// Create the settings for the body itself. Note that here you can also set other properties like the restitution / friction.
	BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	// Create the actual rigid body
	Body *floor = body_interface.CreateBody(floor_settings); // Note that if we run out of bodies this can return nullptr

	// Add it to the world
	body_interface.AddBody(floor->GetID(), EActivation::DontActivate);

    // Now create a dynamic box to bounce on the floor
    // Note that this uses the shorthand version of creating and adding a body to the world
	BodyCreationSettings sphere_settings(new SphereShape(0.5f), RVec3(0.0_r, 2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	BodyID sphere_id = body_interface.CreateAndAddBody(sphere_settings, EActivation::Activate);

	// Now you can interact with the dynamic body, in this case we're going to give it a velocity.
	// (note that if we had used CreateBody then we could have set the velocity straight on the body before adding it to the physics system)
	body_interface.SetLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));

    engineState.SetScene(scene);
    engineState.SetCamera(camera);

    Renderer::Init();

	physics_system.OptimizeBroadPhase();

    // render loop
    // -----------
    while (!window.ShouldClose())
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ClearScreen();

        float cDeltaTime = 1.0f / 60.0f;

		RVec3 positionOfSphere = body_interface.GetCenterOfMassPosition(sphere_id);

        Quat rotationOfSphere = body_interface.GetRotation(sphere_id);

        float x = rotationOfSphere.GetX();
        float y = rotationOfSphere.GetY();
        float z = rotationOfSphere.GetZ();
        float w = rotationOfSphere.GetW();
        // Sync box position and rotation with Jolt Physics
        scene.Get<Transform>(ent)->position = glm::vec3(positionOfSphere.GetX(), positionOfSphere.GetY(), positionOfSphere.GetZ());

        glm::quat quatRotation(x, y, z, w);
        glm::vec3 eulerRotation = glm::eulerAngles(quatRotation); // Converts quaternion to Euler angles

        scene.Get<Transform>(ent)->rotation = eulerRotation;  // Assign to glm::vec3 rotation
        
        UpdateSystems();
        window.Update();

        physics_system.Update(cDeltaTime, 1, &temp_allocator, &job_system);
    }

    // Remove the sphere from the physics system. Note that the sphere itself keeps all of its state and can be re-added at any time.
	body_interface.RemoveBody(sphere_id);

	// Destroy the sphere. After this the sphere ID is no longer valid.
	body_interface.DestroyBody(sphere_id);

	// Remove and destroy the floor
	body_interface.RemoveBody(floor->GetID());
	body_interface.DestroyBody(floor->GetID());

	// Unregisters all types with the factory and cleans up the default material
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

    return 0;
}
