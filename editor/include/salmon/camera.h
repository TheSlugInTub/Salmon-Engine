#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Courtesy of https://learnopengl.com

// Enum for camera movement
enum class CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Camera class for the engine
class Camera
{
  public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f, float pitch = 0.0f, float fov = 90.0f);

    // Moves the camera based on the direction enum
    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    // Rotates the camera based on the xoffset, and yoffset (mouse x and y)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    // Zooms the camera up and down depending on the yoffset (mouse scroll)
    void ProcessMouseScroll(float yoffset);

    // Logs the position, front, up and right vectors
    void PrintValues();

    // Gets the view matrix (glm::lookAt)
    glm::mat4 GetViewMatrix();
    // Gets the projection matrix (glm::perspective)
    glm::mat4 GetProjMatrix(float aspectRatio);
    // Gets the zoom level of the camera
    float GetZoom();
    // Converts screen coordinates to world coordinates
    glm::vec2 ScreenToWorld2D(const glm::vec2& pos);

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float FOV;

  private:
    void updateCameraVectors();

    // Up direction (acts as a constant)
    glm::vec3 WorldUp;
};
