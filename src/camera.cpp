#include <camera.h>
#include <iostream>
#include <utils.h>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    MovementSpeed(2.5f),
    MouseSensitivity(0.1f),
    Zoom(45.0f) 
{
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
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

    if (constrainPitch) {
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

glm::mat4 Camera::GetProjMatrix() 
{
    return glm::perspective(glm::radians(45.0f), Utils::globalWindow->GetAspectRatio(), 0.1f, 100.0f);
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
