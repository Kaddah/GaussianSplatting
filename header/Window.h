#pragma once

#include "D3D12Renderer.h" // Include the D3D12Renderer header
#include <Windows.h>
#include <memory>
#include <d3dx12.h>

class Window
{
public:
  Window(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd);
  ~Window();

  void Stop();
  void Render();
  void ResizeWindow(int width, int height);
  void mainloop();

  CD3DX12_CPU_DESCRIPTOR_HANDLE getRTVHandle();
  std::unique_ptr<D3D12Renderer>& getRenderer() { return renderer; }

private:
  bool                          InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen, LPCWSTR windowName);
  CD3DX12_CPU_DESCRIPTOR_HANDLE getRTVHandle();

  int  _width;
  int  _height;
  HWND _hwnd;
  bool _running;
  bool _fullScreen;

  std::unique_ptr<D3D12Renderer> renderer; // Use unique_ptr to manage D3D12Renderer instance
};

// WndProc function declaration
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
