#pragma once

// Input functions

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

// Key input functions
bool GetKeyDown(Key key);
bool GetKey(Key key);

// Mouse input functions
bool GetMouseButtonDown(MouseKey mouseKey);
bool GetMouseButton(MouseKey mouseKey);

// Mouse cursor input  functions
double GetMouseInputHorizontal();
double GetMouseInputVertical();

}