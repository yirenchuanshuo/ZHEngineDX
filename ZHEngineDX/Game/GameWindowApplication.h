#pragma once
#include"Game.h"

inline wchar_t DebugToDisplay[128] = L"Hello, Windows!";


class GameRHI;
class GameWindowApplication
{
public:
    static int Run(GameRHI* game, HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return g_hwnd; }
    
protected:
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND g_hwnd;
};