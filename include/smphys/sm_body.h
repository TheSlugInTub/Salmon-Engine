#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <chrono>

const glm::vec3 gravity = glm::vec3(0.0f, -5.0f, 0.0f);

struct SmBody
{
	// Holds the inverse mass of the body (-normalMass)
	float inverseMass = 1.0f;

	glm::vec3 position = glm::vec3(0.0f);

	// Orientation of the body, quaternion
	glm::quat orientation;

	glm::vec3 velocity = glm::vec3(0.0f);

	glm::vec3 rotation = glm::vec3(0.0f);

	glm::mat4 transformMatrix;

	glm::mat3 inverseInertiaTensor;

	glm::mat3 inverseInertiaTensorWorld; // World space inverse inertia tensor

	glm::vec3 forceAccum = glm::vec3(0.0f);
	glm::vec3 torqueAccum = glm::vec3(0.0f);

	float angularDamping = 0.0995f;
	float linearDamping = 0.0995f;

	glm::vec3 lastFrameAcceleration = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);
};

struct SmParticle
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);

	glm::vec3 forceAccum = glm::vec3(0.0f);

	// The damping force of the particle
	float damping = 0.8f;

	float inverseMass = 1.0f;
};

inline void ParticleAddForce(SmParticle* part, const glm::vec3& force)
{
	part->forceAccum += force;
}

inline glm::vec3 LocalToWorld(const glm::vec3& local, const glm::mat4& transform)
{
    // Convert glm::vec3 to glm::vec4 by setting w to 1
    glm::vec4 localHomogeneous(local, 1.0f);

    // Apply the transformation matrix
    glm::vec4 worldHomogeneous = transform * localHomogeneous;

    // Convert back to glm::vec3
    return glm::vec3(worldHomogeneous);
}

inline glm::vec3 WorldToLocal(const glm::vec3& world, const glm::mat4& transform)
{
	glm::mat4 inverseTransform;
	inverseTransform = glm::inverse(transform);

    glm::vec4 worldHomogeneous(world, 1.0f);

	glm::vec4 localHomogeneous = transform * worldHomogeneous;
	return glm::vec3(localHomogeneous);
}

inline glm::vec3 TransformDirection(const glm::mat4 &matrix, const glm::vec3 &vector) 
{
    // Extract the upper 3x3 part of the matrix and multiply by the vector
    return glm::vec3(
        vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0],
        vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1],
        vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2]
    );
}

inline glm::vec3 TransformInverseDirection(const glm::mat4 &matrix, const glm::vec3 &vector) {
    // Perform the multiplication by the transposed 3x3 upper-left part of the matrix
    return glm::vec3(
        vector.x * matrix[0][0] + vector.y * matrix[0][1] + vector.z * matrix[0][2],
        vector.x * matrix[1][0] + vector.y * matrix[1][1] + vector.z * matrix[1][2],
        vector.x * matrix[2][0] + vector.y * matrix[2][1] + vector.z * matrix[2][2]
    );
}

inline glm::vec3 LocalToWorldDirn(const glm::vec3& local, const glm::mat4& transform)
{
	glm::vec3 newVec;
	return TransformDirection(transform, local);
}

inline glm::vec3 WorldToLocalDirn(const glm::vec3& world, const glm::mat4& transform)
{
	glm::vec3 newVec;
	return TransformInverseDirection(transform, world);
}

inline glm::mat4 CreateTransformMatrix(const glm::vec3 &position, const glm::quat &orientation) {
    // Convert the quaternion to a rotation matrix
    glm::mat4 rotationMatrix = glm::toMat4(orientation);

    // Create a translation matrix from the position vector
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

    // Combine the translation and rotation matrices
    glm::mat4 transformMatrix = translationMatrix * rotationMatrix;

    return transformMatrix;
}

inline void CalculateDerivedData(SmBody& body)
{
	body.transformMatrix = CreateTransformMatrix(body.position, body.orientation);
}

static inline void _transformInertiaTensor(glm::mat3 &iitWorld,
                                           const glm::quat &q,
                                           const glm::mat3 &iitBody,
                                           const glm::mat4 &rotmat)
{
    // Extract the rotation part of the 4x4 matrix (upper-left 3x3)
    glm::mat3 rotationMatrix = glm::mat3(rotmat);

    // Perform the transformation of the inertia tensor
    glm::mat3 tempMatrix;

    tempMatrix[0] = rotationMatrix[0] * iitBody[0][0] + rotationMatrix[1] * iitBody[0][1] + rotationMatrix[2] * iitBody[0][2];
    tempMatrix[1] = rotationMatrix[0] * iitBody[1][0] + rotationMatrix[1] * iitBody[1][1] + rotationMatrix[2] * iitBody[1][2];
    tempMatrix[2] = rotationMatrix[0] * iitBody[2][0] + rotationMatrix[1] * iitBody[2][1] + rotationMatrix[2] * iitBody[2][2];

    // Final multiplication to get iitWorld
    iitWorld[0] = tempMatrix[0] * rotationMatrix[0][0] + tempMatrix[1] * rotationMatrix[0][1] + tempMatrix[2] * rotationMatrix[0][2];
    iitWorld[1] = tempMatrix[0] * rotationMatrix[1][0] + tempMatrix[1] * rotationMatrix[1][1] + tempMatrix[2] * rotationMatrix[1][2];
    iitWorld[2] = tempMatrix[0] * rotationMatrix[2][0] + tempMatrix[1] * rotationMatrix[2][1] + tempMatrix[2] * rotationMatrix[2][2];
}

inline void SetInertiaTensor(SmBody& body, glm::mat3& inertiaTensor)
{
	_transformInertiaTensor(body.inverseInertiaTensorWorld, body.orientation, body.inverseInertiaTensor, body.transformMatrix);
}

inline void BodyAddForce(SmBody* body, glm::vec3 force)
{
	body->forceAccum += force;
}

inline glm::vec3 VectorProduct(const glm::vec3 &vector, const glm::vec3 &self)
{
	return glm::vec3(self.y*vector.z-self.z*vector.y,
		self.z*vector.x-self.x*vector.z,
		self.x*vector.y-self.y*vector.x);
}

inline void BodyAddForceAtPoint(SmBody& body, const glm::vec3& force, const glm::vec3& point)
{
    // Convert to coordinates relative to center of mass.
    glm::vec3 pt = point;
    pt -= body.position;

    body.forceAccum += force;
    body.torqueAccum += VectorProduct(force, pt);
}

inline void BodyAddForceAtBodyPoint(SmBody& body, const glm::vec3& force, const glm::vec3& point)
{
    // Convert the 3D point to a 4D vector by adding a 1.0 in the w component (homogeneous coordinate).
    glm::vec4 point4D(point, 1.0f);

    // Multiply the 4D point by the transformation matrix.
    glm::vec4 transformedPoint4D = body.transformMatrix * point4D;

    // Convert back to a 3D vector by dropping the w component.
    glm::vec3 transformedPoint = glm::vec3(transformedPoint4D);

    // Apply force at the transformed point.
    BodyAddForceAtPoint(body, force, transformedPoint);
}