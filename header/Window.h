#pragma once

// Standard Library Includes
#include <chrono>
#include <memory>

// Windows and Direct3D Includes
#include <D3Dcompiler.h>
#include <Windows.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <windef.h>
#include <wrl/client.h>

// External Libraries
#include <glm/glm.hpp>

// Project-Specific Includes
#include "ImguiAdapter.h"
#include <Vertex.h>

using Microsoft::WRL::ComPtr;

constexpr int frameBufferCount = 3; // number of buffers (2 = double buffering, 3 = tripple buffering)

struct HfovxyFocal
{
  float htany;
  float htanx;
  float focal;
};

struct ConstantBuffer
{
  glm::mat4   rotationMat;
  glm::mat4   projectionMat;
  glm::mat4   viewMat;
  HfovxyFocal hfovxy_focal;
  glm::mat4   transformMat;
};

class Window
{
public:
  // Constructor and Destructor
  Window(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd);
  virtual ~Window();

  // Pure Virtual Functions
  virtual void                   draw()                                              = 0;
  virtual std::vector<Vertex>    prepareTriangle()                                   = 0;
  virtual void                   drawUI()                                            = 0;
  virtual std::vector<VertexPos> prepareIndices(const std::vector<Vertex>& vertices) = 0;

  // Public Methods
  void Stop();
  void WaitForPreviousFrame();
  void Render();
  void mainloop();
  void UpdateConstantBuffer(const glm::mat4& rotationMat, const glm::mat4& projectionMatrix, const glm::mat4& viewMat,
                            HfovxyFocal hfovxy_focal, const glm::mat4& transformMat);
  void UpdateVertexBuffer(const std::vector<Vertex>& vertices);
  bool InitializeVertexBuffer(const std::vector<Vertex>& vertices);
  void InitializeComputeBuffer(const std::vector<Vertex>& vertices);
  void ResizeWindow(int width, int height);
  CD3DX12_CPU_DESCRIPTOR_HANDLE getRTVHandle();

  // Camera Methods
  void UpdateCameraPosition();
  void UpdateCameraDirection();
  void UpdateRotationFromMouse();
  void OrbitalCamera();
  void InitializeMousePosition();

  // Camera Variables
  POINT prevMousePosCameraDirection = {0, 0};
  POINT prevMousePosRotation        = {0, 0};
  POINT prevMousePosCameraFocus     = {0, 0};

  float alphaX = 0.0f;
  float alphaY = 0.0f;
  float alphaZ = 0.0f;

  float theta  = 0.0f;
  float phi    = 0.0f;
  float radius = 5.0f; // Initial radius of orbital camera

  const float mouseSensX = 0.005f;
  const float mouseSensY = 0.005f;

  glm::vec3 cameraPos    = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::vec3 cameraFront  = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp     = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

  float cameraSpeed = 1.0f;  // Camera speed in meters per second
  float fov         = 45.0f; // Initial zoom level (FOV)
  float nearPlane   = 0.1f;
  float farPlane    = 100.0f;

  float yaw   = -90.0f; // Initialize to face towards negative z-axis
  float pitch = 0.0f;

protected:
  // Protected Members
  int  width_;
  int  height_;
  bool running_;
  bool fullScreen_;
  HWND hwnd_;

  ComPtr<ID3D12Device>         device;
  ComPtr<IDXGISwapChain3>      swapChain;         // Swapchain used to switch between render targets
  ComPtr<ID3D12CommandQueue>   commandQueue;      // Container for command lists
  ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // A descriptor heap to hold resources like the render targets

  ComPtr<ID3D12Resource> renderTargets[frameBufferCount]; // Number of render targets equal to buffer count
  ComPtr<ID3D12CommandAllocator>
      commandAllocator[frameBufferCount];        // Enough allocators for each buffer * number of threads
  ComPtr<ID3D12GraphicsCommandList> commandList; // Add commands, execute to render the frame
  ComPtr<ID3D12Fence>
      fence[frameBufferCount]; // An object that is locked while our command list is being executed by the GPU
  ComPtr<ID3D12Resource>       constantBuffer[frameBufferCount];
  ComPtr<ID3D12DescriptorHeap> cbvHeap;

  // Compute Shader Pipeline
  ComPtr<ID3D12PipelineState>       computePipelineState;
  ComPtr<ID3D12RootSignature>       computeRootSignature;
  ComPtr<ID3D12CommandAllocator>    computeCommandAllocator;
  ComPtr<ID3D12GraphicsCommandList> computeCommandList;

  HANDLE fenceEvent;                   // A handle to an event when our fence is unlocked by the GPU
  UINT64 fenceValue[frameBufferCount]; // This value is incremented each frame. Each fence will have its own value
  int    frameIndex;                   // Current RTV we are on
  int rtvDescriptorSize; // Size of the RTV descriptor on the device (all front and back buffers will be the same size)
  ID3D12PipelineState* pipelineStateObject; // PSO containing a pipeline state
  ID3D12RootSignature* rootSignature;       // Root signature defines data shaders will access
  D3D12_VIEWPORT       viewport;            // Area that output from rasterizer will be stretched to.
  D3D12_RECT           scissorRect;         // The area to draw in. Pixels outside that area will not be drawn onto

  std::unique_ptr<ImGuiAdapter> imguiAdapter;

  std::chrono::high_resolution_clock::time_point before;
  std::chrono::high_resolution_clock::time_point before2;

  // Protected Methods
  bool InitD3D();
  bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen, LPCWSTR windowName);
  void UpdatePipeline();
  void ExecuteComputeShader();

  // Vertex Buffer
  ID3D12Resource* vertexBuffer; // A default buffer in GPU memory that we will load vertex data for our triangle into
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

  bool orbiCam = false;
};
