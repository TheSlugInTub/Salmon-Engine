#include <smphys/sm_body.h>
#include <ecs.h>
#include <scene_manager.h>
#include <utils.h>
#include <components.h>

void SmBodySys()
{
    // Iterate over all SmBodies
	for (EntityID ent : SceneView<SmBody>(*SceneManager::currentScene))
	{
		auto body = SceneManager::currentScene->Get<SmBody>(ent);
		
		body->lastFrameAcceleration = body->acceleration;
		body->lastFrameAcceleration += body->forceAccum * body->inverseMass;

		glm::vec3 angularAcceleration = body->torqueAccum * body->inverseInertiaTensorWorld;

		body->velocity += body->lastFrameAcceleration * Utils::deltaTime;

		body->rotation += angularAcceleration * Utils::deltaTime;

		body->velocity *= powf(body->linearDamping, Utils::deltaTime); 
		body->rotation *= powf(body->angularDamping, Utils::deltaTime); 

		body->position += body->velocity * Utils::deltaTime;

	    glm::quat rotationQuat = glm::quat(glm::vec3(body->rotation * Utils::deltaTime));

	    // Apply the rotation to the current orientation using quaternion multiplication
	    body->orientation = glm::normalize(rotationQuat * body->orientation);
		
		body->velocity *= powf(body->linearDamping, Utils::deltaTime); 
		body->rotation *= powf(body->angularDamping, Utils::deltaTime); 

		CalculateDerivedData(*body);		

		body->forceAccum = glm::vec3(0.0f);
		body->torqueAccum = glm::vec3(0.0f);

		auto trans = SceneManager::currentScene->Get<Transform>(ent);
		trans->position = body->position;
		trans->rotation = body->rotation;
	}

    // Iterate over all SmParticles
	for (EntityID ent : SceneView<SmParticle>(*SceneManager::currentScene))
	{
		assert(Utils::deltaTime > 0.0f && "You messed up big time bruv!");
		auto part = SceneManager::currentScene->Get<SmParticle>(ent);
		
		// Update linear position
		part->position += part->velocity * Utils::deltaTime;

		// Calculate acceleration based on force
		glm::vec3 resultingAcc = part->acceleration;
		resultingAcc += part->forceAccum * part->inverseMass;

		// Update velocity based on acceleration
		part->velocity += resultingAcc * Utils::deltaTime;
		
		// Impose drag onto the velocity
		part->velocity *= powf(part->damping, Utils::deltaTime);

		part->forceAccum = glm::vec3(0.0f);
		
		auto trans = SceneManager::currentScene->Get<Transform>(ent);

		trans->position = part->position;
	}
}

REGISTER_SYSTEM(SmBodySys);