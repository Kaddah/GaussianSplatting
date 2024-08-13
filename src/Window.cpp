#include "Window.h"
#include "D3D12Renderer.h"

extern std::vector<Vertex> vertices;

Window::Window(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd)
    : _width(width)
    , _height(height)
    , _hwnd(NULL)
    , _running(true)
    , _fullScreen(fullScreen)
    , renderer(std::make_unique<D3D12Renderer>(width, height)) // Initialize renderer
{
  if (!InitializeWindow(hInstance, nShowCmd, fullScreen, WindowName))
  {
    throw std::runtime_error("Failed to initialize window");
  }
  if (!renderer->InitD3D(_hwnd))
  {
    MessageBox(0, L"Failed to initialize direct3d 12", L"Error", MB_OK);
  }
  ShowWindow(_hwnd, SW_SHOW);
}

void Window::Stop()
{
  _running = false;
}

Window::~Window()
{
  // Cleanup
  renderer->WaitForPreviousFrame();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Window::getRTVHandle()
{
  return CD3DX12_CPU_DESCRIPTOR_HANDLE(renderer->GetRTVHandle());
}

void Window::ResizeWindow(int width, int height)
{
  _width  = width;
  _height = height;

  renderer->Resize(width, height);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    return true;

  Window* window = nullptr;
  if (msg == WM_NCCREATE)
  {
    CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    window                = reinterpret_cast<Window*>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
  }
  else
  {
    window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  if (window)
  {
    Camera& camera = window->getRenderer()->GetCameraReference();

    switch (msg)
    {
      case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
          if (MessageBox(0, L"Are you sure you want to exit?", L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
          {
            window->Stop();
            DestroyWindow(hwnd);
          }
        }
        return 0;

      case WM_SIZE:
        if (window)
        {
          int width  = LOWORD(lParam);
          int height = HIWORD(lParam);
          window->ResizeWindow(width, height);
        }
        return 0;

      case WM_MOUSEWHEEL:
      {
        if (camera.getOrbiCam())
        {
          int delta = GET_WHEEL_DELTA_WPARAM(wParam);
          camera.ZoomCamera(delta); // Adjust zoom
        }
        return 0;
      }
      break;

      case WM_DESTROY:
        window->Stop();
        PostQuitMessage(0);
        return 0;
    }
  }
  
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Window::InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen, LPCWSTR windowName)
{
  if (fullscreen)
  {
    HMONITOR    hmon = MonitorFromWindow(NULL, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi   = {sizeof(mi)};
    GetMonitorInfo(hmon, &mi);

    _width  = mi.rcMonitor.right - mi.rcMonitor.left;
    _height = mi.rcMonitor.bottom - mi.rcMonitor.top;
  }

  WNDCLASSEX wc;

  wc.cbSize        = sizeof(WNDCLASSEX);
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra    = NULL;
  wc.cbWndExtra    = NULL;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = windowName;
  wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wc))
  {
    MessageBox(NULL, L"Error registering class", L"Error", MB_OK | MB_ICONERROR);
    return false;
  }

  _hwnd = CreateWindowEx(NULL, windowName, windowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, _width,
                         _height, NULL, NULL, hInstance, this);

  if (!_hwnd)
  {
    MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
    return false;
  }

  if (fullscreen)
  {
    SetWindowLong(_hwnd, GWL_STYLE, 0);
  }

  return true;
}

void Window::Render()
{
  renderer->Render();
}

void Window::mainloop()
{
  MSG msg;
  ZeroMemory(&msg, sizeof(MSG));

  renderer->InitializeVertexBuffer(vertices);
  renderer->UpdateVertexBuffer(vertices);

  while (_running)
  {
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT)
      {
        _running = false;
        break;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if (!_running)
    {
      break;
    }

    Render();
  }
}

//CD3DX12_CPU_DESCRIPTOR_HANDLE Window::getRTVHandle()
//{
//  return CD3DX12_CPU_DESCRIPTOR_HANDLE(renderer->GetRTVHandle());
//}