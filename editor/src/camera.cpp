#include <salmon/camera.h>
#include <salmon/engine.h>
#include <iostream>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch, float fov)
   : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f)
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    FOV = fov;
    updateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    if (direction == CameraMovement::FORWARD)
        Position += Front * velocity;
    if (direction == CameraMovement::BACKWARD)
        Position -= Front * velocity;
    if (direction == CameraMovement::LEFT)
        Position -= Right * velocity;
    if (direction == CameraMovement::RIGHT)
        Position += Right * velocity;
    if (direction == CameraMovement::UP)
        Position += Up * velocity;
    if (direction == CameraMovement::DOWN)
        Position -= Up * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    if (Zoom >= 1.0f && Zoom <= 45.0f)
        Zoom -= yoffset;
    if (Zoom <= 1.0f)
        Zoom = 1.0f;
    if (Zoom >= 45.0f)
        Zoom = 45.0f;
}

void Camera::PrintValues()
{
    std::cout << Position.x << ", " << Position.y << ", " << Position.z << '\n';
    glm::vec3 newPosition = Position + Front;
    std::cout << newPosition.x << ", " << newPosition.y << ", " << newPosition.z << '\n';
    std::cout << Up.x << ", " << Up.y << ", " << Up.z << '\n';
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjMatrix(float aspectRatio)
{
    return glm::perspective(glm::radians(FOV), aspectRatio, 0.1f, 500.0f);
}

float Camera::GetZoom()
{
    return Zoom;
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::vec2 Camera::ScreenToWorld2D(const glm::vec2& pos)
{
    // Convert screen coordinates to Normalized Device Coordinates (NDC)
    glm::vec4 rayStart_NDC(
        ((float)pos.x / (float)engineState.window->width - 0.5f) * 2.0f,
        ((float)pos.y / (float)engineState.window->height - 0.5f) * 2.0f,
        -1.0f, // Near plane
        1.0f);
    glm::vec4 rayEnd_NDC(
        ((float)pos.x / (float)engineState.window->width - 0.5f) * 2.0f,
        ((float)pos.y / (float)engineState.window->height - 0.5f) * 2.0f,
        0.0f, // Far plane
        1.0f);

    // Compute the inverse of the combined projection and view matrix
    glm::mat4 invM = glm::inverse(engineState.projMat * GetViewMatrix());

    // Transform NDC coordinates to world coordinates
    glm::vec4 rayStart_world = invM * rayStart_NDC;
    rayStart_world /= rayStart_world.w;
    glm::vec4 rayEnd_world = invM * rayEnd_NDC;
    rayEnd_world /= rayEnd_world.w;

    float planeZ = 0.0f;

    // Compute the direction of the ray in world space
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayEnd_world - rayStart_world));

    // Calculate the intersection of the ray with the 2D plane
    float t = (planeZ - rayStart_world.z) / rayDir.z;
    glm::vec2 result = glm::vec2(glm::vec3(rayStart_world) + t * rayDir);
    result = glm::vec2(result.x, -result.y);
    result.y += Position.y * 2 ;
    return result;
}
