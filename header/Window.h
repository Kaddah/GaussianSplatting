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
  Window(LPCTSTR WindowName,
         int     width, // of window
         int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd);

  virtual void draw() = 0;
  // virtual std::vector<Vertex> prepareTriangle()=0;
  virtual std::vector<Vertex> prepareTriangle() = 0;
  void                        Stop();
  void                        WaitForPreviousFrame();
  void                        Render();
  void                        mainloop();
  virtual void drawUI() = 0;
  void UpdateConstantBuffer(const glm::mat4& rotationMat, const glm::mat4& projectionMatrix, const glm::mat4& viewMat,
                            HfovxyFocal hfovxy_focal, const glm::mat4& transformMat);
  virtual std::vector<VertexPos> prepareIndices(const std::vector<Vertex>& vertices) = 0;

  void UpdateVertexBuffer(const std::vector<Vertex>& vertices);
  bool InitializeVertexBuffer(const std::vector<Vertex>& vertices);
  void InitializeComputeBuffer(const std::vector<Vertex>& vertices);
  bool InitializeIndexBuffer(const std::vector<uint32_t>& indices);
  

  void ResizeWindow(int width, int height);

  ~Window();

  CD3DX12_CPU_DESCRIPTOR_HANDLE getRTVHandle();
  ID3D12Resource* vertexBuffer; // a default buffer in GPU memory that we will load vertex data for our triangle into
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
  ComPtr<ID3D12Resource>   indexBuffer;
  D3D12_INDEX_BUFFER_VIEW indexBufferView;

  void UpdateCameraPosition();
  void UpdateCameraDirection();
  void UpdateRotationFromMouse();
  void OrbitalCamera();
  void InitializeMousePosition();

 // enum class CameraMode
 // {
 //   Normal,
 //   Orbital
 // };
  //CameraMode cameraMode = CameraMode::Normal;

  bool orbiCam = false;


  POINT prevMousePosCameraDirection = {0, 0};
  POINT prevMousePosRotation        = {0, 0};
  POINT prevMousePosCameraFocus     = {0, 0};

  //
  float alphaX  = 0.0f;
  float alphaY  = 0.0f;
  float alphaZ  = 0.0f;

  float theta   = 0.0f;
  float phi     = 0.0f;
  float radius  = 5.0f; //initial radius of orbitalcamera

  const float mouseSensX = 0.005f;
  const float mouseSensY = 0.005f;
  // Camera position and movement variables
  glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 5.0f);
  glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
  glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

  float cameraSpeed = 1.0f;  // Camera speed in meters per second
  float fov         = 45.0f; // Initial zoom level (FOV)

  float nearPlane = 0.1f;
  float farPlane  = 100.0f;

  float yaw   = -90.0f; // Initialize to face towards negative z-axis
  float pitch = 0.0f;

protected:
  int  _width;
  int  _height;
  bool _running;
  bool _fullScreen;
  HWND _hwnd;
  //  POINT                 prevMousePosRotation        = {0, 0};
  // POINT                 prevMousePosCameraDirection = {0, 0};
  ComPtr<ID3D12Device>         device;
  ComPtr<IDXGISwapChain3>      swapChain;         // swapchain used to switch between render targets
  ComPtr<ID3D12CommandQueue>   commandQueue;      // container for command lists
  ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap; // a descriptor heap to hold resources like the render targets

  ComPtr<ID3D12Resource> renderTargets[frameBufferCount]; // number of render targets equal to buffer count
  ComPtr<ID3D12CommandAllocator>
      commandAllocator[frameBufferCount];        // enough allocators for each buffer * number of threads
  ComPtr<ID3D12GraphicsCommandList> commandList; // add commands, execute to render the frame
  ComPtr<ID3D12Fence>
      fence[frameBufferCount]; // an object that is locked while our command list is being executed by the gpu
  ComPtr<ID3D12Resource>       constantBuffer[frameBufferCount];
  ComPtr<ID3D12DescriptorHeap> cbvHeap;

  // compute shader pipeline
  ComPtr<ID3D12PipelineState>       computePipelineState;
  ComPtr<ID3D12RootSignature>       computeRootSignature;
  ComPtr<ID3D12CommandAllocator>    computeCommandAllocator;
  ComPtr<ID3D12GraphicsCommandList> computeCommandList;

  HANDLE fenceEvent;                   // a handle to an event when our fence is unlocked by the gpu
  UINT64 fenceValue[frameBufferCount]; // this value is incremented each frame. each fence will have its own value
  int    frameIndex;                   // current rtv we are on
  int rtvDescriptorSize; // size of the rtv descriptor on the device (all front and back buffers will be the same size)
  ID3D12PipelineState* pipelineStateObject; // pso containing a pipeline state
  ID3D12RootSignature* rootSignature;       // root signature defines data shaders will access
  D3D12_VIEWPORT       viewport;            // area that output from rasterizer will be stretched to.
  D3D12_RECT           scissorRect;         // the area to draw in. pixels outside that area will not be drawn onto

  std::unique_ptr<ImGuiAdapter> imguiAdapter;

  std::chrono::high_resolution_clock::time_point before;
  std::chrono::high_resolution_clock::time_point before2;

  bool InitD3D();
  bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, bool fullscreen, LPCWSTR windowName);

  void UpdatePipeline();
  void ExecuteComputeShader();
};
