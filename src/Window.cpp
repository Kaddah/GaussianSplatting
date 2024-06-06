#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <stdexcept>
#include <wrl/client.h>

#include "Window.h"
#include "WipImgui.h"
#include "d3dx12.h"
#include <DxException.h>
#include <GaussianRenderer.h>
#include <Shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct ConstantBuffer
{
  glm::mat4 rotationMat;
};

std::vector<Vertex> quaVerti;
size_t              vBufferSize;

Window::Window(LPCTSTR WindowName, int width, int height, bool fullScreen, HINSTANCE hInstance, int nShowCmd)
    : _width(width)
    , _height(height)
    , _hwnd(NULL)
    , _running(true)
    , _fullScreen(fullScreen) // initializer list
{
  if (!InitializeWindow(hInstance, nShowCmd, fullScreen, WindowName))
  {
    throw std::runtime_error("Failed to initialize window");
  }
  if (!InitD3D())
  {
    MessageBox(0, L"Failed to initialize direct3d 12", L"Error", MB_OK);
  }
}

bool Window::InitD3D()
{
  HRESULT hr;

// Enable Debug Layer
#ifndef NDEBUG
  ComPtr<ID3D12Debug> debugController;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
  {
    debugController->EnableDebugLayer();
    std::cout << "Debug layer ENABLED" << std::endl;
  }
#endif

  // Create Device
  IDXGIFactory4* dxgiFactory;
  ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

  IDXGIAdapter1* adapter; // adapter = graphics card

  int  adapterIndex = 0;
  bool adapterFound = false;

  // find first hardware gpu that supports d3d 12
  while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
  {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
    {
      continue;
    }

    // -> device that is compatible with direct3d 12
    hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
    if (SUCCEEDED(hr))
    {
      adapterFound = true;
      break;
    }

    adapterIndex++;
  }

  if (!adapterFound)
  {
    return false;
  }

  // Create device
  ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

  // Create a direct command queue
  D3D12_COMMAND_QUEUE_DESC cqDesc = {};
  cqDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cqDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

  ThrowIfFailed(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue))); // create the command queue

  // Create the Swap Chain (double/tripple buffering)
  DXGI_MODE_DESC backBufferDesc = {};
  backBufferDesc.Width          = _width;
  backBufferDesc.Height         = _height;
  backBufferDesc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;

  // multisampling -> no multisampling -> value = 1
  DXGI_SAMPLE_DESC sampleDesc = {};
  sampleDesc.Count            = 1; // multisample count

  // Describe and create the swap chain.
  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferCount          = frameBufferCount;
  swapChainDesc.BufferDesc           = backBufferDesc;
  swapChainDesc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT; // pipeline will render to this swap chain
  swapChainDesc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after call present
  swapChainDesc.OutputWindow = _hwnd;
  swapChainDesc.SampleDesc   = sampleDesc;
  swapChainDesc.Windowed     = !_fullScreen;

  IDXGISwapChain* tempSwapChain;

  dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, &tempSwapChain);

  swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);

  frameIndex = swapChain->GetCurrentBackBufferIndex();

  // Create the Back Buffers (render target views) Descriptor Heap
  //  describe an rtv descriptor heap and create
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors             = frameBufferCount;
  rtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

  // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from
  // the pipeline otherwise set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  hr                = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
  if (FAILED(hr))
  {
    return false;
  }

  // get the size of a descriptor in this heap
  rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  // get a handle to the first descriptor in the descriptor heap (handle is like pointer)
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

  // Create a RTV for each buffer
  for (int i = 0; i < frameBufferCount; i++)
  {
    // get the n'th buffer in the swap chain and store it in the n'th position of our ID3D12Resource array
    hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
    if (FAILED(hr))
    {
      return false;
    }

    //"create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
    device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);

    // increment the rtv handle by the rtv descriptor size above
    rtvHandle.Offset(1, rtvDescriptorSize);
  }

  // Create the Command Allocators
  for (int i = 0; i < frameBufferCount; i++)
  {
    hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
    if (FAILED(hr))
    {
      return false;
    }
  }

  // create the command list with the first allocator
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex].Get(), NULL,
                                 IID_PPV_ARGS(&commandList));
  if (FAILED(hr))
  {
    return false;
  }

  // create the fences
  for (int i = 0; i < frameBufferCount; i++)
  {
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
    if (FAILED(hr))
    {
      return false;
    }
    fenceValue[i] = 0; // set the initial fence value to 0
  }

  // create a handle to a fence event
  fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (fenceEvent == nullptr)
  {
    return false;
  }

  CD3DX12_ROOT_PARAMETER parameter = {};
  parameter.InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
  // create root signature
  rootSignatureDesc.Init(1, &parameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ID3DBlob* signature;
  hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
  if (FAILED(hr))
  {
    return false;
  }

  hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                   IID_PPV_ARGS(&rootSignature));
  if (FAILED(hr))
  {
    return false;
  }

  // create vertex and pixel shaders
  // compile vertex shader
   ID3DBlob* vertexShader = nullptr; // d3d blob for holding vertex shader bytecode
   ID3DBlob* errorBuff = nullptr;    // a buffer holding the error data if any
   std::wcout << L"Compiling vertex shader...\n";
   if (!CompileShader(L"../shader/VertexShader.hlsl", ShaderType::Vertex, &vertexShader, &errorBuff))
   {
     std::cerr << "Failed to compile vertex shader." << std::endl;
   }
    
  // fill out a shader bytecode structure (which is basically just a pointer to the shader bytecode and the size of the
  // shader bytecode)
  D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
  vertexShaderBytecode.BytecodeLength        = vertexShader->GetBufferSize();
  vertexShaderBytecode.pShaderBytecode       = vertexShader->GetBufferPointer();

  // compile pixel shader
  ID3DBlob* pixelShader = nullptr;
  std::wcout << L"Compiling pixel shader...\n";
  if (!CompileShader(L"../shader/PixelShader.hlsl", ShaderType::Pixel, &pixelShader, &errorBuff))
  {
    std::cerr << "Failed to compile pixel shader." << std::endl;
  }

  // fill out shader bytecode structure for pixel shader
  D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
  pixelShaderBytecode.BytecodeLength        = pixelShader->GetBufferSize();
  pixelShaderBytecode.pShaderBytecode       = pixelShader->GetBufferPointer();

  // compile geometry shader
  ID3DBlob* geometryShader = nullptr;
  std::wcout << L"Compiling geometry shader...\n";
  if (!CompileShader(L"../shader/GeometryShader.hlsl", ShaderType::Geometry, &geometryShader, &errorBuff))
  {
    std::cerr << "Failed to compile geometry shader." << std::endl;
  }

  // fill out shader bytecode structure for geometry shader
  D3D12_SHADER_BYTECODE geometryShaderBytecode = {};
  geometryShaderBytecode.BytecodeLength        = geometryShader->GetBufferSize();
  geometryShaderBytecode.pShaderBytecode       = geometryShader->GetBufferPointer();

  // create input layout
  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos),
                                             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                                            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal),
                                             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                                            {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color),
                                             D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

  // D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
  //     {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  //     {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}, // Correct
  //     offset if different
  // };

  // fill out an input layout description structure
  D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};

  inputLayoutDesc.NumElements        = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
  inputLayoutDesc.pInputElementDescs = inputLayout;

  // create a pipeline state object (PSO)
  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout                        = inputLayoutDesc;
  psoDesc.pRootSignature                     = rootSignature;
  psoDesc.VS                                 = vertexShaderBytecode;
  psoDesc.PS                                 = pixelShaderBytecode;
  psoDesc.GS                                 = geometryShaderBytecode;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
  psoDesc.RTVFormats[0]         = DXGI_FORMAT_R8G8B8A8_UNORM;             // format of the render target
  psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
  psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
  psoDesc.RasterizerState  = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.BlendState       = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.NumRenderTargets = 1;

  // create the pso
  ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)));

  // Create vertex buffer
  // Vertex vList[] = {{glm::vec3(0.0f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
  //                  {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)},
  //                  {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)}};

  // execute the command list to upload the initial assets (triangle data)

  // create constant buffer
  for (int i = 0; i < frameBufferCount; i++)
  {
    const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    const auto constantBufferDesc   = CD3DX12_RESOURCE_DESC::Buffer(sizeof(ConstantBuffer));
    ThrowIfFailed(device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &constantBufferDesc,
                                                  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                  IID_PPV_ARGS(&constantBuffer[i])));
  }

  commandList->Close();
  ID3D12CommandList* ppCommandLists[] = {commandList.Get()};
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // increment the fence value now, otherwise the buffer might not be uploaded by the time we start drawing
  fenceValue[frameIndex]++;
  hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
  if (FAILED(hr))
  {
    _running = false;
  }

  if (SUCCEEDED(hr))
  {
    imguiAdapter = new ImGuiAdapter(device, frameBufferCount, _hwnd);
  }
  else
  {
    return false;
  }

  // Fill out the Viewport
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width    = _width;
  viewport.Height   = _height;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

  // Fill out a scissor rect
  scissorRect.left   = 0;
  scissorRect.top    = 0;
  scissorRect.right  = _width;
  scissorRect.bottom = _height;

  return true;
}

void Window::Stop()
{
  _running = false;
}

Window::~Window()
{
  // Cleanup
  //  wait for the gpu to finish all frames
  WaitForPreviousFrame();
  // get swapchain out of full screen before exiting
  BOOL fs = false;
  if (swapChain->GetFullscreenState(&fs, NULL))
    swapChain->SetFullscreenState(false, NULL);
  CloseHandle(fenceEvent);
}

CD3DX12_CPU_DESCRIPTOR_HANDLE Window::getRTVHandle()
{
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex,
                                          rtvDescriptorSize);
  return rtvHandle;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    return true;

  Window* window = nullptr;
  if (msg == WM_NCCREATE)
  {
    // Set the pointer to window instance
    CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    window                = reinterpret_cast<Window*>(pCreate->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
  }
  else
  {
    // Retrieve the pointer to window instance
    window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

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

    case WM_DESTROY: // x button on top right corner of window was pressed
      window->Stop();
      PostQuitMessage(0);
      return 0;
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

  _hwnd =
      CreateWindowEx(NULL, windowName,
                     windowName, // windowTitle
                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, _width, _height, NULL, NULL, hInstance, this);

  if (!_hwnd)
  {
    MessageBox(NULL, L"Error creating window", L"Error", MB_OK | MB_ICONERROR);
    return false;
  }

  if (fullscreen)
  {
    SetWindowLong(_hwnd, GWL_STYLE, 0);
  }

  ShowWindow(_hwnd, ShowWnd);
  UpdateWindow(_hwnd);

  return true;
}

void Window::Render()
{
  HRESULT hr;

  UpdatePipeline(100.0f, 0.1f); // update the pipeline by sending commands to the commandqueue
  // create an array of command lists (only one command list here)
  ID3D12CommandList* ppCommandLists[] = {commandList.Get()};

  // execute the array of command lists
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
  if (FAILED(hr))
  {
    _running = false;
  }

  // present the current backbuffer
  hr = swapChain->Present(0, 0);
  if (FAILED(hr))
  {
    _running = false;
  }
}

void Window::mainloop()
{
  MSG msg;
  ZeroMemory(&msg, sizeof(MSG));

  quaVerti = prepareTriangle();

  InitializeVertexBuffer(quaVerti);

  UpdateVertexBuffer(quaVerti);

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

    Render(); // execute the command queue
  }
}
// rotation variables and mouse sensitivity
static float alphaX = 0.0f;
static float alphaY = 0.0f;
static float alphaZ = 0.0f;

const float mouseSensX = 0.005f;
const float mouseSensY = 0.005f;

// Store previous mouse position
static POINT prevMousePos = {0, 0};
void         UpdateRotationFromMouse()
{
  // Check if mouse is hovering over ImGui window
  if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive())
  {
    return;
  }
  if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) // Check if left mouse button is held down
  {
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);

    // Calculate the mouse movement delta
    int deltaX = currentMousePos.x - prevMousePos.x;
    int deltaY = currentMousePos.y - prevMousePos.y;

    // Update rotation angles based on mouse movement
    alphaY += deltaX * mouseSensX; // Rotate around Y-axis with horizontal mouse movement
    alphaX += deltaY * mouseSensY; // Rotate around X-axis with vertical mouse movement
    // clamp angles
    alphaX = glm::mod(alphaX, glm::two_pi<float>());
    alphaY = glm::mod(alphaY, glm::two_pi<float>());
    // Update previous mouse position
    prevMousePos = currentMousePos;
  }
  else
  {
    // Update previous mouse position when button is not pressed to avoid sudden jumps
    GetCursorPos(&prevMousePos);
  }
}
// Call function to initialize previous mouse pos
void InitializeMousePosition()
{
  GetCursorPos(&prevMousePos);
}
void Window::UpdatePipeline(float angle, float aspectRatio)
{
  HRESULT hr;

  // wait for the gpu to finish with the command allocator before we reset it
  WaitForPreviousFrame();

  // only reset an allocator once the gpu is done with it. resetting an allocator frees the memory that the command list
  // was stored in
  ThrowIfFailed(commandAllocator[frameIndex]->Reset());

  // reset the command list
  ThrowIfFailed(commandList->Reset(commandAllocator[frameIndex].Get(), pipelineStateObject));

  UpdateRotationFromMouse();

  // Create individual rotation matrices for each axis
  glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), alphaX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), alphaY, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), alphaZ, glm::vec3(0.0f, 0.0f, 1.0f));

  // Combine the rotations
  glm::mat4 rotationMat = rotationZ * rotationY * rotationX;

  // Update the constant buffer with the combined rotation matrix
  UpdateConstantBuffer(rotationMat);
  // Call this onc  to set the initial mouse position
  InitializeMousePosition();
  // recording commands into the commandList (which all the commands will be stored in the commandAllocator)
  //  transition the "frameIndex" render target from the present state to the render target state so the command list
  //  draws to it starting from here
  auto resBarrierTransition = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
  commandList->ResourceBarrier(1, &resBarrierTransition);

  draw();
  // imgui command list update here

  imguiAdapter->startMainImGui();
  // ImGui::ShowDemoWindow();
  imguiAdapter->createWindow(alphaX, alphaY, alphaZ);
  imguiAdapter->renderImGui();
  imguiAdapter->commandList(commandList);

  // transition the "frameIndex" render target from the render target state to the present state
  auto resBarrierTransPresent = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  commandList->ResourceBarrier(1, &resBarrierTransPresent);

  ThrowIfFailed(commandList->Close());
}

void Window::WaitForPreviousFrame()
{
  HRESULT hr;

  // swap the current rtv buffer index so we draw on the correct buffer
  frameIndex = swapChain->GetCurrentBackBufferIndex();

  // if the current fence value is still less than "fenceValue", then we know the GPU has not finished executing
  // the command queue since it has not reached the "commandQueue->Signal(fence, fenceValue)" command
  if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
  {
    // fence create an event which is signaled once the fence's current value is "fenceValue"
    hr = fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
    if (FAILED(hr))
    {
      _running = false;
    }

    // wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
    // has reached "fenceValue", we know the command queue has finished executing
    WaitForSingleObject(fenceEvent, INFINITE);
  }

  // increment fenceValue for next frame
  fenceValue[frameIndex]++;
}

void Window::UpdateVertexBuffer(const std::vector<Vertex>& vertices)
{
  if (!vertexBuffer)
  {
    throw std::runtime_error("Vertex buffer is not initialized.");
  }

  if (vertices.empty())
  {
    throw std::runtime_error("Vertex data is empty.");
  }

  size_t newValue = vertices.size() * sizeof(Vertex);

  Vertex* vertexDataBegin = nullptr;
  HRESULT hr              = vertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataBegin));
  if (FAILED(hr))
  {
    throw std::runtime_error("Failed to map vertex buffer.");
  }

  memcpy(vertexDataBegin, vertices.data(), newValue);
  vertexBuffer->Unmap(0, nullptr);

  vertexBufferView.SizeInBytes = static_cast<UINT>(newValue);
}

bool Window::InitializeVertexBuffer(const std::vector<Vertex>& vertices)
{
  try
  {
    vBufferSize = vertices.size() * sizeof(Vertex);
    // Create default heap for the vertex buffer
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc   = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

    HRESULT hr =
        device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create vertex buffer.");
    }
    vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    // Create upload heap for vertex buffer
    auto            heapPropertiesUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ID3D12Resource* vBufferUploadHeap;
    hr = device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vBufferUploadHeap));
    if (FAILED(hr))
    {
      throw std::runtime_error("Failed to create vertex buffer upload heap.");
    }
    vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

    // Copy vertex data to upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData                  = vertices.data();
    vertexData.RowPitch               = vBufferSize;
    vertexData.SlicePitch             = vBufferSize;

    // Schedule copy from upload heap to default heap
    UpdateSubresources(commandList.Get(), vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

    // Transition vertex buffer to vertex buffer state
    auto resBarrierVertexBuffer = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                       D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    commandList->ResourceBarrier(1, &resBarrierVertexBuffer);

    // Execute the command list to upload vertex data
    commandList->Close();
    ID3D12CommandList* ppCommandLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Increment fence value
    fenceValue[frameIndex]++;
    hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
    if (FAILED(hr))
    {
      _running = false;
      return false;
    }

    // Create vertex buffer view
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes  = sizeof(Vertex);
    vertexBufferView.SizeInBytes    = vBufferSize;

    return true;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error initializing vertex buffer: " << e.what() << std::endl;
    return false;
  }
}

void Window::UpdateConstantBuffer(const glm::mat4& rotationMat)
{
  if (!constantBuffer[frameIndex])
  {
    throw std::runtime_error("Constant buffer is not initialized.");
  }
  ConstantBuffer* cbDataBegin = nullptr;
  ThrowIfFailed(constantBuffer[frameIndex]->Map(0, nullptr, reinterpret_cast<void**>(&cbDataBegin)));
  cbDataBegin->rotationMat = rotationMat; // Update the MVP matrix in the constant buffer
  constantBuffer[frameIndex]->Unmap(0, nullptr);
}
