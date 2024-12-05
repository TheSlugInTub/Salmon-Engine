#include <salmon/ecs.h>
#include <salmon/engine.h>
#include <salmon/input.h>
#include <salmon/components.h>

void CameraMoveSys()
{
    if (glfwGetKey(engineState.window->window, GLFW_KEY_W) == GLFW_PRESS)
        engineState.camera->ProcessKeyboard(CameraMovement::UP, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_S) == GLFW_PRESS)
        engineState.camera->ProcessKeyboard(CameraMovement::DOWN, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_A) == GLFW_PRESS)
        engineState.camera->ProcessKeyboard(CameraMovement::LEFT, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_D) == GLFW_PRESS)
        engineState.camera->ProcessKeyboard(CameraMovement::RIGHT, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_E) == GLFW_PRESS)
        engineState.camera->ProcessKeyboard(CameraMovement::FORWARD, engineState.deltaTime);
    if (glfwGetKey(engineState.window->window, GLFW_KEY_Q) == GLFW_PRESS)
        engineState.camera->ProcessKeyboard(CameraMovement::BACKWARD, engineState.deltaTime);
}

void CameraLookSys()
{
    if (Input::GetMouseButton(MouseKey::MiddleClick))
    {
        // Variables to store the last mouse position
        static bool  firstMouse = true;
        static float lastX = 0.0f, lastY = 0.0f;

        // Get the current mouse position
        double xpos, ypos;
        glfwGetCursorPos(engineState.window->window, &xpos, &ypos);

        if (firstMouse)
        {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            firstMouse = false;
        }

        // Calculate the offset from the last position
        float xoffset = static_cast<float>(xpos) - lastX;
        float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates go from bottom to top

        // Update the last position
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);

        // Process the mouse movement with the calculated offset
        engineState.camera->ProcessMouseMovement(xoffset, yoffset);
    }
}

REGISTER_SYSTEM(CameraMoveSys);
REGISTER_SYSTEM(CameraLookSys);
