#pragma once
#include "ImguiAdapter.h"
#include <D3Dcompiler.h>
#include <Vertex.h>
#include <Windows.h>
#include <chrono>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <glm/glm.hpp>
#include <memory>
#include <windef.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

constexpr int frameBufferCount = 3; // number of buffers (2 = double buffering, 3 = tripple buffering)

class Window
{
public:
  // Constructor and Destructor
  Window(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd);
  ~Window();

  // Pure virtual functions to be implemented by derived classes
  virtual void                   draw()                                              = 0;
  virtual void                   drawUI()                                            = 0;
  virtual std::vector<Vertex>    prepareTriangle()                                   = 0;
  virtual std::vector<VertexPos> prepareIndices(const std::vector<Vertex>& vertices) = 0;

  // Main Loop
  void mainloop();
  void Render();
  void Stop();
  void ResizeWindow(int width, int height);

protected:
  // Initialization Functions
  bool InitD3D();
  bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen, LPCWSTR windowName);

  // Resource Management
  void WaitForPreviousFrame();
  void UpdatePipeline();
  void ExecuteComputeShader();
  void CleanupResources();

  // Buffer Management
  void                          UpdateConstantBuffer(const glm::mat4& rotationMat);
  void                          UpdateVertexBuffer(const std::vector<Vertex>& vertices);
  bool                          InitializeVertexBuffer(const std::vector<Vertex>& vertices);
  void                          InitializeComputeBuffer(const std::vector<Vertex>& vertices);
  CD3DX12_CPU_DESCRIPTOR_HANDLE getRTVHandle();

  // Input Handling
  void UpdateCameraPosition();
  void UpdateCameraDirection();
  void UpdateRotationFromMouse();
  void InitializeMousePosition();

  // Member Variables
  int  _width;
  int  _height;
  bool _running;
  bool _fullScreen;
  HWND _hwnd;

  // DirectX 12 Objects
  ComPtr<ID3D12Device>              device;
  ComPtr<IDXGISwapChain3>           swapChain;
  ComPtr<ID3D12CommandQueue>        commandQueue;
  ComPtr<ID3D12DescriptorHeap>      rtvDescriptorHeap;
  ComPtr<ID3D12Resource>            renderTargets[frameBufferCount];
  ComPtr<ID3D12CommandAllocator>    commandAllocator[frameBufferCount];
  ComPtr<ID3D12GraphicsCommandList> commandList;
  ComPtr<ID3D12Fence>               fence[frameBufferCount];
  ComPtr<ID3D12Resource>            constantBuffer[frameBufferCount];
  ComPtr<ID3D12DescriptorHeap>      cbvHeap;
  ComPtr<ID3D12PipelineState>       computePipelineState;
  ComPtr<ID3D12RootSignature>       computeRootSignature;
  ComPtr<ID3D12CommandAllocator>    computeCommandAllocator;
  ComPtr<ID3D12GraphicsCommandList> computeCommandList;
  ComPtr<ID3D12Resource>            vertexBuffer;
  D3D12_VERTEX_BUFFER_VIEW          vertexBufferView;

  // Fence and Synchronization
  HANDLE               fenceEvent;
  UINT64               fenceValue[frameBufferCount];
  int                  frameIndex;
  int                  rtvDescriptorSize;
  ID3D12PipelineState* pipelineStateObject;
  ID3D12RootSignature* rootSignature;
  D3D12_VIEWPORT       viewport;
  D3D12_RECT           scissorRect;

  // ImGui Adapter
  std::unique_ptr<ImGuiAdapter> imguiAdapter;

  // Camera and Input Variables
  POINT       prevMousePosCameraDirection = {0, 0};
  POINT       prevMousePosRotation        = {0, 0};
  float       alphaX                      = 0.0f;
  float       alphaY                      = 0.0f;
  float       alphaZ                      = 0.0f;
  const float mouseSensX                  = 0.005f;
  const float mouseSensY                  = 0.005f;
  glm::vec3   cameraPos                   = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::vec3   cameraFront                 = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3   cameraUp                    = glm::vec3(0.0f, 1.0f, 0.0f);
  float       cameraSpeed                 = 1.0f;
  float       fov                         = 45.0f;
  float       nearPlane                   = 0.1f;
  float       farPlane                    = 100.0f;
  float       yaw                         = -90.0f;
  float       pitch                       = 0.0f;

  // Timing Variables
  std::chrono::high_resolution_clock::time_point before;
  std::chrono::high_resolution_clock::time_point before2;
};
