#pragma once
#include "d3d12.h"

class DrawWindow {

    HWND hwnd = NULL;
    bool isRunning = true;
    LPCTSTR WindowName = L"Dreieck";
    LPCTSTR WindowTitle = L"Dreieck";

    int mWidth = 800;
    int mHeight = 600;

public:
    DrawWindow() = default;
    ~DrawWindow();


    bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen);

    LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



    inline bool IsRunning() { return isRunning; }


    
};