#pragma once

#include "Vertex.h"
#include <d3d12.h>
#include <glm/glm.hpp>
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Window;

class D3D12Renderer
{
public:
  D3D12Renderer();
  ~D3D12Renderer();

  bool InitD3D(Window* window);
  void Render();
  void UpdatePipeline();

private:
  bool CompileShadersAndCreatePSO();
  void ExecuteComputeShader();
  void WaitForPreviousFrame();

  // D3D12 components
  ComPtr<ID3D12Device>              device;
  ComPtr<ID3D12CommandQueue>        commandQueue;
  ComPtr<IDXGISwapChain3>           swapChain;
  ComPtr<ID3D12DescriptorHeap>      rtvDescriptorHeap;
  ComPtr<ID3D12Resource>            renderTargets[2];
  ComPtr<ID3D12CommandAllocator>    commandAllocator[2];
  ComPtr<ID3D12GraphicsCommandList> commandList;
  ComPtr<ID3D12PipelineState>       pipelineStateObject;
  ComPtr<ID3D12RootSignature>       rootSignature;
  ComPtr<ID3D12Fence>               fence[2];
  HANDLE                            fenceEvent;
  UINT64                            fenceValue[2];
  UINT                              frameIndex;
  UINT                              rtvDescriptorSize;
  D3D12_VIEWPORT                    viewport;
  D3D12_RECT                        scissorRect;

  // Compute Shader components
  ComPtr<ID3D12CommandQueue>        computeCommandQueue;
  ComPtr<ID3D12CommandAllocator>    computeCommandAllocator;
  ComPtr<ID3D12GraphicsCommandList> computeCommandList;
  ComPtr<ID3D12PipelineState>       computePipelineState;
  ComPtr<ID3D12DescriptorHeap>      uavHeap;
  ComPtr<ID3D12Resource>            positionBuffer;
  std::vector<VertexPos>            positions;
};