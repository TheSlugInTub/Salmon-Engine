#include <input.h>
#include <unordered_map>
#include <map>
#include <GLFW/glfw3.h>

namespace Input
{

std::unordered_map<Key, int> keyMap = {
    { Key::A, GLFW_KEY_A },
    { Key::B, GLFW_KEY_B },
    { Key::C, GLFW_KEY_C },
    { Key::D, GLFW_KEY_D },
    { Key::E, GLFW_KEY_E },
    { Key::F, GLFW_KEY_F },
    { Key::G, GLFW_KEY_G },
    { Key::H, GLFW_KEY_H },
    { Key::I, GLFW_KEY_I },
    { Key::J, GLFW_KEY_J },
    { Key::K, GLFW_KEY_K },
    { Key::L, GLFW_KEY_L },
    { Key::M, GLFW_KEY_M },
    { Key::N, GLFW_KEY_N },
    { Key::O, GLFW_KEY_O },
    { Key::P, GLFW_KEY_P },
    { Key::Q, GLFW_KEY_Q },
    { Key::R, GLFW_KEY_R },
    { Key::S, GLFW_KEY_S },
    { Key::T, GLFW_KEY_T },
    { Key::U, GLFW_KEY_U },
    { Key::V, GLFW_KEY_V },
    { Key::W, GLFW_KEY_W },
    { Key::X, GLFW_KEY_X },
    { Key::Y, GLFW_KEY_Y },
    { Key::Z, GLFW_KEY_Z },
    { Key::Num0, GLFW_KEY_0 },
    { Key::Num1, GLFW_KEY_1 },
    { Key::Num2, GLFW_KEY_2 },
    { Key::Num3, GLFW_KEY_3 },
    { Key::Num4, GLFW_KEY_4 },
    { Key::Num5, GLFW_KEY_5 },
    { Key::Num6, GLFW_KEY_6 },
    { Key::Num7, GLFW_KEY_7 },
    { Key::Num8, GLFW_KEY_8 },
    { Key::Num9, GLFW_KEY_9 },
    { Key::Escape, GLFW_KEY_ESCAPE },
    { Key::Tab, GLFW_KEY_TAB },
    { Key::Space, GLFW_KEY_SPACE },
    { Key::RShift, GLFW_KEY_RIGHT_SHIFT },
    { Key::LShift, GLFW_KEY_RIGHT_SHIFT },
    { Key::RCtrl, GLFW_KEY_RIGHT_CONTROL },
    { Key::LCtrl, GLFW_KEY_LEFT_CONTROL },
    { Key::Left, GLFW_KEY_LEFT},
    { Key::Right, GLFW_KEY_RIGHT},
    { Key::Up, GLFW_KEY_UP},
    { Key::Down, GLFW_KEY_DOWN},
    { Key::Delete, GLFW_KEY_DELETE },
};

std::unordered_map<MouseKey, int> mouseKeyMap = {
    { MouseKey::LeftClick, GLFW_MOUSE_BUTTON_LEFT },
    { MouseKey::MiddleClick, GLFW_MOUSE_BUTTON_MIDDLE },
    { MouseKey::RightClick, GLFW_MOUSE_BUTTON_RIGHT }
};

// bool GetKeyDown(Key key) {
//     static std::map<Key, bool> keyState;
//     static std::map<Key, bool> keyStatePrev;
// 
//     int glfwKey = keyMap[key];
//     keyStatePrev[key] = keyState[key];
//     keyState[key] = glfwGetKey(Utils::globalWindow->window, glfwKey) == GLFW_PRESS;
// 
//     return keyState[key] && !keyStatePrev[key];
// }
// 
// bool GetKey(Key key)
// {
//     auto it = keyMap.find(key);
//     if (it != keyMap.end())
//     {
//         int glfwKey = it->second;
//         return glfwGetKey(Utils::globalWindow->window, glfwKey) == GLFW_PRESS;
//     }
//     return false;
// }
// 
// bool GetMouseButtonDown(MouseKey mouseKey) {
//     static std::map<MouseKey, bool> mouseButtonState;
//     static std::map<MouseKey, bool> mouseButtonStatePrev;
// 
//     int glfwMouseButton = mouseKeyMap[mouseKey];
//     mouseButtonStatePrev[mouseKey] = mouseButtonState[mouseKey];
//     mouseButtonState[mouseKey] = glfwGetMouseButton(Utils::globalWindow->window, glfwMouseButton) == GLFW_PRESS;
// 
//     return mouseButtonState[mouseKey] && !mouseButtonStatePrev[mouseKey];
// }
// 
// bool GetMouseButton(MouseKey mouseKey)
// {
//     auto it = mouseKeyMap.find(mouseKey);
//     if (it != mouseKeyMap.end())
//     {
//         int glfwMouseButton = it->second;
//         return glfwGetMouseButton(Utils::globalWindow->window, glfwMouseButton) == GLFW_PRESS;
//     }
//     return false;
// }
// 
// double GetMouseInputHorizontal()
// {
//     double mouseX, mouseY;
// 
//     glfwGetCursorPos(Utils::globalWindow->window, &mouseX, &mouseY);
// 
//     return mouseX;
// }
// 
// double GetMouseInputVertical()
// {
//     double mouseX, mouseY;
// 
//     glfwGetCursorPos(Utils::globalWindow->window, &mouseX, &mouseY);
// 
//     return mouseY;
// }

}
