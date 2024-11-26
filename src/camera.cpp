#include <salmon/camera.h>
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

glm::vec3 Camera::ScreenToWorld(int windowWidth, int windowHeight, const glm::ivec2& pos)
{
    float ar = (float)windowWidth / (float)windowHeight;

    glm::vec2 viewportSize(windowHeight, windowHeight);

    float zDepth = Position.z * 1.87f;

    glm::mat4 projection = glm::perspective(glm::radians(GetZoom()), ar, 0.1f, 100.0f);
    glm::mat projInverse = glm::inverse(projection);

    float mouse_x = (float)pos.x;
    float mouse_y = (float)pos.y;

    float ndc_x = (2.0f * mouse_x) / windowWidth - 1.0f;
    float ndc_y = 1.0f - (2.0f * mouse_y) / windowHeight;

    double focal_length = 1.0f / glm::tan(glm::radians(45.0f / 2.0f));
    glm::vec3 ray_view(ndc_x / focal_length, (ndc_y * ar) / focal_length, 1.0f);

    glm::vec4 ray_ndc_4d(ndc_x, ndc_y, 1.0f, 1.0f);
    glm::vec4 ray_view_4d = projInverse * ray_ndc_4d;

    glm::vec4 view_space_intersect = glm::vec4(ray_view * zDepth, 1.0f);

    glm::mat4 view = GetViewMatrix();
    glm::mat4 viewInverse = glm::inverse(view);

    glm::vec4 point_world = viewInverse * view_space_intersect;

    return glm::vec3(point_world);
}
