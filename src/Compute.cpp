#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>
#include <iostream>
#include <wrl.h>

using namespace Microsoft::WRL;

// Helper function to throw an exception on failure
inline void ThrowIfFailed(HRESULT hr)
{
  if (FAILED(hr))
  {
    throw std::exception();
  }
}

// Constants
const UINT FrameCount        = 3;
const UINT ThreadGroupCountX = 8;
const UINT ThreadGroupCountY = 8;
const UINT ThreadGroupCountZ = 1;

// Structures
struct Vertex
{
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT4 color;
};

// Global variables
ComPtr<ID3D12Device>              g_device;
ComPtr<ID3D12CommandQueue>        g_commandQueue;
ComPtr<IDXGISwapChain3>           g_swapChain;
ComPtr<ID3D12DescriptorHeap>      g_rtvHeap;
ComPtr<ID3D12Resource>            g_renderTargets[FrameCount];
ComPtr<ID3D12CommandAllocator>    g_commandAllocators[FrameCount];
ComPtr<ID3D12GraphicsCommandList> g_commandList;
ComPtr<ID3D12Fence>               g_fence;
UINT64                            g_fenceValue = 0;
HANDLE                            g_fenceEvent;

// Function prototypes
bool InitializeDirect3D(HWND hWnd);
void CreateRenderTargetViews();
void PopulateCommandList();
void WaitForPreviousFrame();

int main()
{
  // Window initialization (assumed to be handled in your main script)
  HWND hwnd = GetConsoleWindow(); // Replace with your window handle

  // DirectX 12 initialization
  if (!InitializeDirect3D(hwnd))
  {
    std::cerr << "Direct3D initialization failed." << std::endl;
    return 1;
  }

  // Compute Shader initialization (example)
  // Compile Compute Shader
  ID3DBlob* computeShaderBlob = nullptr;
  ID3DBlob* errorBlob         = nullptr;
  if (FAILED(D3DCompileFromFile(L"ComputeShader.hlsl", nullptr, nullptr, "CSMain", "cs_5_0", D3DCOMPILE_DEBUG, 0,
                                &computeShaderBlob, &errorBlob)))
  {
    if (errorBlob)
    {
      OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
      errorBlob->Release();
    }
    return 1;
  }

  // Create Compute Pipeline State Object (PSO)
  D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
  computePsoDesc.pRootSignature                    = nullptr; // Replace with your root signature
  computePsoDesc.CS                                = CD3DX12_SHADER_BYTECODE(computeShaderBlob);
  computePsoDesc.Flags                             = D3D12_PIPELINE_STATE_FLAG_NONE;

  ComPtr<ID3D12PipelineState> computePipelineState;
  ThrowIfFailed(g_device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&computePipelineState)));

  // Create UAV Descriptor Heap (if needed for UAVs)
  // ComPtr<ID3D12DescriptorHeap> uavDescriptorHeap;
  // D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
  // ...

  // Example of using the Compute Shader
  {
    // Populate command list with compute shader dispatch
    PopulateCommandList();

    // Execute command list
    g_commandQueue->ExecuteCommandLists(1, CommandListCast(g_commandList.GetAddressOf()));

    // Wait for GPU to finish execution
    WaitForPreviousFrame();
  }

  return 0;
}

bool InitializeDirect3D(HWND hWnd)
{
  // Create DXGI factory
  ComPtr<IDXGIFactory4> dxgiFactory;
  ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));

  // Create device
  ComPtr<IDXGIAdapter1> adapter;
  ThrowIfFailed(dxgiFactory->EnumAdapters1(0, &adapter));
  ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_device)));

  // Create command queue
  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Type                     = D3D12_COMMAND_LIST_TYPE_DIRECT;
  queueDesc.Flags                    = D3D12_COMMAND_QUEUE_FLAG_NONE;
  ThrowIfFailed(g_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_commandQueue)));

  // Create swap chain
  DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
  swapChainDesc.BufferCount          = FrameCount;
  swapChainDesc.BufferDesc.Width     = 1280; // Replace with your width
  swapChainDesc.BufferDesc.Height    = 720;  // Replace with your height
  swapChainDesc.BufferDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage          = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect           = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.OutputWindow         = hWnd;
  swapChainDesc.SampleDesc.Count     = 1;
  ThrowIfFailed(dxgiFactory->CreateSwapChain(g_commandQueue.Get(), &swapChainDesc, &g_swapChain));

  // Create descriptor heap for render target views (RTVs)
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
  rtvHeapDesc.NumDescriptors             = FrameCount;
  rtvHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  ThrowIfFailed(g_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&g_rtvHeap)));

  // Create resources for render targets
  CreateRenderTargetViews();

  // Create command allocator for each frame
  for (UINT i = 0; i < FrameCount; ++i)
  {
    ThrowIfFailed(
        g_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_commandAllocators[i])));
  }

  // Create command list
  ThrowIfFailed(g_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0].Get(), nullptr,
                                            IID_PPV_ARGS(&g_commandList)));

  // Create fence and event handle
  ThrowIfFailed(g_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)));
  g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (g_fenceEvent == nullptr)
  {
    ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
  }

  return true;
}

void CreateRenderTargetViews()
{
  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(g_rtvHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < FrameCount; i++)
  {
    ThrowIfFailed(g_swapChain->GetBuffer(i, IID_PPV_ARGS(&g_renderTargets[i])));
    g_device->CreateRenderTargetView(g_renderTargets[i].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, g_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
  }
}

void PopulateCommandList()
{
  // Reset command allocator and command list
  ThrowIfFailed(g_commandAllocators[g_swapChain->GetCurrentBackBufferIndex()]->Reset());
  ThrowIfFailed(g_commandList->Reset(g_commandAllocators[g_swapChain->GetCurrentBackBufferIndex()].Get(), nullptr));

  // Set root signature, pipeline state object, etc. (if applicable)

  // Set compute pipeline state object
  g_commandList->SetPipelineState(/* computePipelineStateObject */);

  // Dispatch
  g_commandList->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

  // Close command list
  ThrowIfFailed(g_commandList->Close());
}

void WaitForPreviousFrame()
{
  // Signal and increment fence value
  const UINT64 fenceToWaitFor = g_fenceValue;
  ThrowIfFailed(g_commandQueue->Signal(g_fence.Get(), fenceToWaitFor));
  g_fenceValue++;

  // Wait until the GPU has completed command execution
  if (g_fence->GetCompletedValue() < fenceToWaitFor)
  {
    ThrowIfFailed(g_fence->SetEventOnCompletion(fenceToWaitFor, g_fenceEvent));
    WaitForSingleObject(g_fenceEvent, INFINITE);
  }

  // Present the frame
  ThrowIfFailed(g_swapChain->Present(1, 0));
}
