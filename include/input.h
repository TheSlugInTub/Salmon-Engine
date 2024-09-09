#pragma once

// Input functions

// Key enum for specifying keyboard input
enum class Key
{
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Escape, Tab, LShift, RShift, LCtrl, RCtrl, Space,
    Left, Right, Up, Down, Delete
};

enum class MouseKey
{
    LeftClick, RightClick, MiddleClick
};

namespace Input
{

// Only returns true on the first frame that a key is pressed
bool GetKeyDown(Key key);
// Returns true if the key is pressed
bool GetKey(Key key);

// Much like the key functions but with mouse input

bool GetMouseButtonDown(MouseKey mouseKey);
bool GetMouseButton(MouseKey mouseKey);

// These functions get the screen-space position of the mouse

double GetMouseInputHorizontal();
double GetMouseInputVertical();

}
