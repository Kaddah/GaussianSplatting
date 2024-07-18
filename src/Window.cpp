#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <chrono>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <stdexcept>
#include <wrl/client.h>

#include "ImguiAdapter.h"
#include "Window.h"
#include "d3dx12.h"
#include <DxException.h>
#include <GaussianRenderer.h>
#include <Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math_extensions.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <memory>
#include <PlyReader.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct ConstantBuffer
{
  glm::mat4 rotationMat;
};


std::vector<Vertex> quaVerti;
std::vector<VertexPos> vertIndex;

ComPtr<ID3D12DescriptorHeap> uavHeap;
ComPtr<ID3D12Resource>       positionBuffer;
std::vector<VertexPos>       positions;
ComPtr<ID3D12CommandQueue>   computeCommandQueue;

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
  ComPtr<IDXGIFactory4> dxgiFactory;
  ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

  ComPtr<IDXGIAdapter1> adapter;
  int                   adapterIndex = 0;
  bool                  adapterFound = false;

  while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
  {
    DXGI_ADAPTER_DESC1 desc;
    adapter->GetDesc1(&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
    {
      adapterIndex++;
      continue;
    }

    hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
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

  ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

  // Create a direct command queue
  D3D12_COMMAND_QUEUE_DESC cqDesc = {};
  cqDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
  cqDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;

  ThrowIfFailed(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)));

  // Create compute command queue
  D3D12_COMMAND_QUEUE_DESC computeCqDesc = {};
  computeCqDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
  computeCqDesc.Type                     = D3D12_COMMAND_LIST_TYPE_COMPUTE;

  ThrowIfFailed(device->CreateCommandQueue(&computeCqDesc, IID_PPV_ARGS(&computeCommandQueue)));

  // Create the Swap Chain
  DXGI_MODE_DESC backBufferDesc = {};
  backBufferDesc.Width          = _width;
  backBufferDesc.Height         = _height;
  backBufferDesc.Format         = DXGI_FORMAT_R8G8B8A8_UNORM;

  DXGI_SAMPLE_DESC sampleDesc = {};
  sampleDesc.Count            = 1;

  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferCount          = frameBufferCount;
  swapChainDesc.BufferDesc           = backBufferDesc;
  swapChainDesc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.OutputWindow         = _hwnd;
  swapChainDesc.SampleDesc           = sampleDesc;
  swapChainDesc.Windowed             = !_fullScreen;

  ComPtr<IDXGISwapChain> tempSwapChain;
  ThrowIfFailed(dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, &tempSwapChain));

  swapChain  = static_cast<IDXGISwapChain3*>(tempSwapChain.Get());
  frameIndex = swapChain->GetCurrentBackBufferIndex();

  // Create the Back Buffers Descriptor Heap
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors             = frameBufferCount;
  rtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap)));

  rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

  for (int i = 0; i < frameBufferCount; i++)
  {
    ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])));
    device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, rtvDescriptorSize);
  }

  for (int i = 0; i < frameBufferCount; i++)
  {
    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i])));
  }

  ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[frameIndex].Get(), NULL,
                                          IID_PPV_ARGS(&commandList)));

  for (int i = 0; i < frameBufferCount; i++)
  {
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i])));
    fenceValue[i] = 0;
  }

  fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (fenceEvent == nullptr)
  {
    return false;
  }

  CD3DX12_ROOT_PARAMETER parameter = {};
  parameter.InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL);

  CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
  rootSignatureDesc.Init(1, &parameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

  ComPtr<ID3DBlob> signature;
  ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr));
  ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
                                            IID_PPV_ARGS(&rootSignature)));

  ID3DBlob* vertexShader = nullptr;
  ID3DBlob* errorBuff    = nullptr;
  std::wcout << L"Compiling vertex shader...\n";
  if (!CompileShader(L"../shader/VertexShader.hlsl", ShaderType::Vertex, &vertexShader, &errorBuff))
  {
    std::cerr << "Failed to compile vertex shader." << std::endl;
  }

  D3D12_SHADER_BYTECODE vertexShaderBytecode = {};
  vertexShaderBytecode.BytecodeLength        = vertexShader->GetBufferSize();
  vertexShaderBytecode.pShaderBytecode       = vertexShader->GetBufferPointer();

  ID3DBlob* pixelShader = nullptr;
  std::wcout << L"Compiling pixel shader...\n";
  if (!CompileShader(L"../shader/PixelShader.hlsl", ShaderType::Pixel, &pixelShader, &errorBuff))
  {
    std::cerr << "Failed to compile pixel shader." << std::endl;
  }

  D3D12_SHADER_BYTECODE pixelShaderBytecode = {};
  pixelShaderBytecode.BytecodeLength        = pixelShader->GetBufferSize();
  pixelShaderBytecode.pShaderBytecode       = pixelShader->GetBufferPointer();

  ID3DBlob* geometryShader = nullptr;
  std::wcout << L"Compiling geometry shader...\n";
  if (!CompileShader(L"../shader/GeometryShader.hlsl", ShaderType::Geometry, &geometryShader, &errorBuff))
  {
    std::cerr << "Failed to compile geometry shader." << std::endl;
  }

  D3D12_SHADER_BYTECODE geometryShaderBytecode = {};
  geometryShaderBytecode.BytecodeLength        = geometryShader->GetBufferSize();
  geometryShaderBytecode.pShaderBytecode       = geometryShader->GetBufferPointer();

  ID3DBlob* computeShader = nullptr;
  std::wcout << L"Compiling Compute shader...\n";
  if (!CompileShader(L"../shader/ComputeShader.hlsl", ShaderType::Compute, &computeShader, &errorBuff))
  {
    std::cerr << "Failed to compile compute shader." << std::endl;
    if (errorBuff)
    {
      std::cerr << (char*)errorBuff->GetBufferPointer() << std::endl;
      errorBuff->Release();
    }
    return false;
  }

  D3D12_SHADER_BYTECODE computeShaderBytecode = {};
  computeShaderBytecode.BytecodeLength        = computeShader->GetBufferSize();
  computeShaderBytecode.pShaderBytecode       = computeShader->GetBufferPointer();

  D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal),
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
       0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 0,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 1,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 2,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 3, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 3,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 4, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 4,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 5, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 5,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 6, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 6,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 7, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 7,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 8, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 8,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 9, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 9,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 10, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 10,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 11, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 11,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 12, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 12,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 13, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 13,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 14, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 14,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 15, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, f_rest) + sizeof(glm::vec3) * 15,
       D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

  D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
  inputLayoutDesc.NumElements             = sizeof(inputLayout) / sizeof(D3D12_INPUT_ELEMENT_DESC);
  inputLayoutDesc.pInputElementDescs      = inputLayout;

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
  psoDesc.InputLayout                        = inputLayoutDesc;
  psoDesc.pRootSignature                     = rootSignature;
  psoDesc.VS                                 = vertexShaderBytecode;
  psoDesc.PS                                 = pixelShaderBytecode;
  psoDesc.GS                                 = geometryShaderBytecode;
  psoDesc.PrimitiveTopologyType              = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
  psoDesc.RTVFormats[0]                      = DXGI_FORMAT_R8G8B8A8_UNORM;
  psoDesc.SampleDesc                         = sampleDesc;
  psoDesc.SampleMask                         = 0xffffffff;
  psoDesc.RasterizerState                    = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  //psoDesc.BlendState                         = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.NumRenderTargets                   = 1;

  // Blend State konfigurieren
  D3D12_RENDER_TARGET_BLEND_DESC rtvBlendDesc = {};
  rtvBlendDesc.BlendEnable                    = TRUE;
  rtvBlendDesc.SrcBlend                       = D3D12_BLEND_SRC_ALPHA;
  rtvBlendDesc.DestBlend                      = D3D12_BLEND_INV_SRC_ALPHA;
  rtvBlendDesc.BlendOp                        = D3D12_BLEND_OP_ADD;
  rtvBlendDesc.SrcBlendAlpha                  = D3D12_BLEND_ONE;
  rtvBlendDesc.DestBlendAlpha                 = D3D12_BLEND_ZERO;
  rtvBlendDesc.BlendOpAlpha                   = D3D12_BLEND_OP_ADD;
  rtvBlendDesc.RenderTargetWriteMask          = D3D12_COLOR_WRITE_ENABLE_ALL;

  psoDesc.BlendState.RenderTarget[0] = rtvBlendDesc;


  ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)));

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

  fenceValue[frameIndex]++;
  hr = commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);
  if (FAILED(hr))
  {
    _running = false;
  }

  // Create root signature for compute shader
  CD3DX12_ROOT_PARAMETER computeRootParams[3];
  //computeRootParams[0].InitAsUnorderedAccessView(0);
  computeRootParams[0].InitAsConstantBufferView(0);  // cbuffer Constants : register(b0)
  computeRootParams[1].InitAsShaderResourceView(0); // StructuredBuffer inputPositions : register(t0)
  computeRootParams[2].InitAsUnorderedAccessView(0); // RWStructuredBuffer outputPositions : register(u0)


  CD3DX12_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
  computeRootSignatureDesc.Init(_countof(computeRootParams), computeRootParams, 0, nullptr,
                                D3D12_ROOT_SIGNATURE_FLAG_NONE);

  ComPtr<ID3DBlob> computeSignature;
  ComPtr<ID3DBlob> computeErrorBlob;
  hr = D3D12SerializeRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &computeSignature,
                                   &computeErrorBlob);
  if (FAILED(hr))
  {
    std::cerr << "Failed to serialize compute root signature." << std::endl;
    if (computeErrorBlob)
    {
      std::cerr << (char*)computeErrorBlob->GetBufferPointer() << std::endl;
      computeErrorBlob->Release();
    }
    return false;
  }

  hr = device->CreateRootSignature(0, computeSignature->GetBufferPointer(), computeSignature->GetBufferSize(),
                                   IID_PPV_ARGS(&computeRootSignature));
  if (FAILED(hr))
  {
    std::cerr << "Failed to create compute root signature." << std::endl;
    return false;
  }


  // Create compute pipeline state object
  D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
  computePsoDesc.pRootSignature                    = computeRootSignature.Get();
  computePsoDesc.CS                                = computeShaderBytecode;
  ThrowIfFailed(device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&computePipelineState)));

  // Create command allocator and command list for compute
  ThrowIfFailed(
      device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&computeCommandAllocator)));
  ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, computeCommandAllocator.Get(),
                                          computePipelineState.Get(), IID_PPV_ARGS(&computeCommandList)));
  computeCommandList->Close();
  
  // Initialize positions for compute shader
  InitializeComputeBuffer(vertices);

  
  if (SUCCEEDED(hr))
  {
    imguiAdapter = std::make_unique<ImGuiAdapter>(device, frameBufferCount, _hwnd);
  }
  else
  {
    return false;
  }

  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width    = _width;
  viewport.Height   = _height;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;

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

  UpdatePipeline(); // update the pipeline by sending commands to the commandqueue
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

  vertIndex = prepareIndices(quaVerti);

  //InitializeComputeBuffer(vertIndex);

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
void Window::UpdateRotationFromMouse()
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
    int deltaX = currentMousePos.x - prevMousePosRotation.x;
    int deltaY = currentMousePos.y - prevMousePosRotation.y;

    // Update rotation angles based on mouse movement
    alphaY += deltaX * mouseSensX; // Rotate around Y-axis with horizontal mouse movement
    alphaX += deltaY * mouseSensY; // Rotate around X-axis with vertical mouse movement
    // clamp angles

    // Update previous mouse position
    prevMousePosRotation = currentMousePos;
  }
  else
  {
    // Update previous mouse position when button is not pressed to avoid sudden jumps
    GetCursorPos(&prevMousePosRotation);
  }
}

// Call function to initialize previous mouse pos
void Window::InitializeMousePosition()
{
  {
    POINT initialMousePos;
    GetCursorPos(&initialMousePos);
    prevMousePosRotation        = initialMousePos;
    prevMousePosCameraDirection = initialMousePos;
  }
}

void Window::UpdateCameraPosition()
{
  // TODO move this variable into Window class as member
  auto  now    = std::chrono::high_resolution_clock::now();
  float deltaS = std::chrono::duration_cast<std::chrono::nanoseconds>(now - before).count() / 1e9f;
  before       = now;
  if (GetAsyncKeyState('W') & 0x8000)
  {
    cameraPos += cameraSpeed * cameraFront * deltaS;
  }
  if (GetAsyncKeyState('S') & 0x8000)
  {
    cameraPos -= cameraSpeed * cameraFront * deltaS; // Move down
  }
  if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) //
  {
    cameraPos -= cameraSpeed * cameraUp * deltaS; // Move down
  }
  if (GetAsyncKeyState(VK_SPACE) & 0x8000) //
  {
    cameraPos += cameraSpeed * cameraUp * deltaS; // Move down
  }

  const glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
  if (GetAsyncKeyState('A') & 0x8000)
  {
    cameraPos -= cameraRight * cameraSpeed * deltaS; // Move left
  }
  if (GetAsyncKeyState('D') & 0x8000)
  {
    cameraPos += cameraRight * cameraSpeed * deltaS; // Move right
  }
}

void Window::UpdateCameraDirection()
{

  if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) // Check if right mouse button is held down
  {
    POINT currentMousePos;
    GetCursorPos(&currentMousePos);

    // Calculate the mouse movement delta
    int deltaX = currentMousePos.x - prevMousePosCameraDirection.x;
    int deltaY = currentMousePos.y - prevMousePosCameraDirection.y;

    // Update camera front vector based on mouse movement
    float sensitivity = 0.05f;
    yaw += deltaX * sensitivity;
    pitch -= deltaY * sensitivity;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    // Update previous mouse position
    prevMousePosCameraDirection = currentMousePos;
  }
  else
  {
    // Update previous mouse position when button is not pressed to avoid sudden jumps
    GetCursorPos(&prevMousePosCameraDirection);
  }
}
void Window::UpdatePipeline()
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
  UpdateCameraPosition();
  UpdateCameraDirection();
  // get aspectratio
  float aspectRatio = static_cast<float>(_width) / static_cast<float>(_height);
  // Create individual rotation matrices for each axis
  glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), alphaX, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), alphaY, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), alphaZ, glm::vec3(0.0f, 0.0f, 1.0f));

  // Combine the rotations
  glm::mat4 rotationMat = rotationZ * rotationY * rotationX;

  glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  // near plane 0.1f farplane 100.0f
  glm::mat4 projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
  // Combine matrices to get mvp
  glm::mat4 mvpMat = projectionMatrix * viewMatrix * rotationMat;

  // Update the constant buffer with mvp
  UpdateConstantBuffer(mvpMat);
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
  imguiAdapter->createWindow(alphaX, alphaY, alphaZ, cameraSpeed, cameraPos, cameraFront, cameraUp);
  imguiAdapter->renderImGui();
  imguiAdapter->commandList(commandList);

  // transition the "frameIndex" render target from the render target state to the present state
  auto resBarrierTransPresent = CD3DX12_RESOURCE_BARRIER::Transition(
      renderTargets[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  commandList->ResourceBarrier(1, &resBarrierTransPresent);

  ThrowIfFailed(commandList->Close());

   // Execute compute shader
  ExecuteComputeShader();
}

void Window::ExecuteComputeShader()
{
  HRESULT hr;

  // Reset compute command allocator and command list
  ThrowIfFailed(computeCommandAllocator->Reset());
  ThrowIfFailed(computeCommandList->Reset(computeCommandAllocator.Get(), computePipelineState.Get()));

  // Set descriptor heaps
  ID3D12DescriptorHeap* ppHeaps[] = {uavHeap.Get()};
  computeCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

  // Transition the position buffer to UAV state
  CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
      positionBuffer.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
  computeCommandList->ResourceBarrier(1, &uavBarrier);

  // Set compute root signature
  computeCommandList->SetComputeRootSignature(computeRootSignature.Get());

  // Set compute shader UAV
  computeCommandList->SetComputeRootUnorderedAccessView(0, positionBuffer->GetGPUVirtualAddress());

  // Dispatch compute shader
  computeCommandList->Dispatch(static_cast<UINT>(ceil(static_cast<float>(positions.size()) / 256.0f)), 1, 1);

  // Transition the position buffer back to UAV state
  uavBarrier = CD3DX12_RESOURCE_BARRIER::Transition(positionBuffer.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                                                    D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  computeCommandList->ResourceBarrier(1, &uavBarrier);

  // Close and execute compute command list
  ThrowIfFailed(computeCommandList->Close());
  ID3D12CommandList* ppCommandLists[] = {computeCommandList.Get()};
  computeCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  // Signal and wait for the compute queue
  ThrowIfFailed(computeCommandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));
  if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
  {
    ThrowIfFailed(fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent));
    WaitForSingleObject(fenceEvent, INFINITE);
  }

  // Increment the fence value for the next frame
  fenceValue[frameIndex]++;

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
    if (vertices.empty())
    {
      throw std::runtime_error("Vertex data is empty.");
    }
    vBufferSize = vertices.size() * sizeof(Vertex);
    // Create default heap for the vertex buffer
    auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto resourceDesc   = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

   ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                  IID_PPV_ARGS(&vertexBuffer)));
    vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    // Create upload heap for vertex buffer
    auto            heapPropertiesUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    ID3D12Resource* vBufferUploadHeap;
    ThrowIfFailed(device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                  D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                  IID_PPV_ARGS(&vBufferUploadHeap)));
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
    ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));

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

void Window::InitializeComputeBuffer(const std::vector<Vertex>& vertices)
{
  // Prepare position and index buffer data
  std::vector<VertexPos> positions(vertices.size());
  for (size_t i = 0; i < vertices.size(); ++i)
  {
    positions[i].position = vertices[i].pos;
    positions[i].index    = static_cast<uint32_t>(i);
  }

  size_t positionBufferSize = positions.size() * sizeof(VertexPos);

 // Create upload heap for initial data
  ComPtr<ID3D12Resource> positionUploadBuffer;
  D3D12_HEAP_PROPERTIES  uploadHeapProps  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  D3D12_RESOURCE_DESC    uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(positionBufferSize);

  ThrowIfFailed(device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                IID_PPV_ARGS(&positionUploadBuffer)));

  // Copy position data to the upload buffer
  void* pData;
  ThrowIfFailed(positionUploadBuffer->Map(0, nullptr, &pData));
  memcpy(pData, positions.data(), positionBufferSize);
  positionUploadBuffer->Unmap(0, nullptr);

  // Create default heap for the position buffer
  D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  D3D12_RESOURCE_DESC   bufferDesc =
      CD3DX12_RESOURCE_DESC::Buffer(positionBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

  ThrowIfFailed(device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                                D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                IID_PPV_ARGS(&positionBuffer)));

  // Copy data from the upload buffer to the default buffer
  commandList->CopyBufferRegion(positionBuffer.Get(), 0, positionUploadBuffer.Get(), 0, positionBufferSize);

  // Transition position buffer to UAV state
  CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
      positionBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  commandList->ResourceBarrier(1, &uavBarrier);

  // Create a descriptor heap for UAVs
  D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
  uavHeapDesc.NumDescriptors             = 1;
  uavHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  uavHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  ThrowIfFailed(device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&uavHeap)));

  // Create UAV for the position buffer
  D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
  uavDesc.Buffer.NumElements               = static_cast<UINT>(positions.size());
  uavDesc.Buffer.StructureByteStride       = sizeof(VertexPos);

  CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(uavHeap->GetCPUDescriptorHandleForHeapStart());
  device->CreateUnorderedAccessView(positionBuffer.Get(), nullptr, &uavDesc, uavHandle);
}
