#pragma once

//#ifndef D3D12_RENDERER_H
//#define D3D12_RENDERER_H

#include "Camera.h"
#include "ImguiAdapter.h" // Include ImguiAdapter header
#include "Vertex.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <dxgi1_4.h> // Include DXGI 1.4 for IDXGISwapChain3
#include <memory>
#include <vector>
#include <wrl/client.h>
#include <wrl.h>
#include "Shader.h"
#include <cmath>


using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct HfovxyFocal
{
  float htany;
  float htanx;
  float focal;
};

struct ConstantBuffer
{
  glm::mat4   transformMat;
  glm::mat4   rotationMat;
  glm::mat4   projectionMat;
  glm::mat4   viewMat;
  HfovxyFocal hfovxy_focal;
};


class D3D12Renderer
{
public:
  D3D12Renderer(int width, int height);
  ~D3D12Renderer();

  bool InitD3D(HWND hwnd);
  void Resize(int width, int height);
  void Render();
  void WaitForPreviousFrame();

  CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle();
  // CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle() const; // Correct the function declaration
  HfovxyFocal                   calculateHfovxyFocal(float fovy, float _height, float _width);

  ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }
  ID3D12RootSignature* GetRootSignature() { return rootSignature.Get(); }
  const D3D12_VIEWPORT& GetViewport() { return viewport; }
  const D3D12_RECT& GetScissorRect() { return scissorRect; }
  const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() { return vertexBufferView; }
  D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress() { return constantBuffer[frameIndex]->GetGPUVirtualAddress(); }
  const std::unique_ptr<Camera>& GetCamera() { return camera; }
  ComPtr<ID3D12DescriptorHeap> GetRTVDescriptorHeap() const { return rtvDescriptorHeap; }
  int GetFrameIndex() const { return frameIndex; }
  UINT GetRTVDescriptorSize() const { return rtvDescriptorSize; }

private:
  void UpdatePipeline();
  void UpdateVertexBuffer(const std::vector<Vertex>& vertices);
  bool InitializeVertexBuffer(const std::vector<Vertex>& vertices);
  void UpdateConstantBuffer(const glm::mat4& rotationMat, const glm::mat4& projectionMat, const glm::mat4& viewMat,
                            HfovxyFocal hfovxy_focal, const glm::mat4& transformMat);
  void InitializeComputeBuffer(const std::vector<Vertex>& vertices);
  void ExecuteComputeShader();
  void draw();
  void drawUI();

private:
  int  _width;
  int  _height;
  bool _running;
  int frameIndex;

  static const int frameBufferCount = 2;

  ComPtr<ID3D12Device>              device;
  ComPtr<ID3D12CommandQueue>        commandQueue;
  ComPtr<IDXGISwapChain3>           swapChain; // Ensure correct IDXGISwapChain3 type
  ComPtr<ID3D12DescriptorHeap>      rtvDescriptorHeap;
  ComPtr<ID3D12Resource>            renderTargets[frameBufferCount];
  ComPtr<ID3D12CommandAllocator>    commandAllocator[frameBufferCount];
  ComPtr<ID3D12GraphicsCommandList> commandList;
  ComPtr<ID3D12PipelineState>       pipelineStateObject;
  ComPtr<ID3D12RootSignature>       rootSignature;
  ComPtr<ID3D12Fence>               fence[frameBufferCount];
  ComPtr<ID3D12Resource>            constantBuffer[frameBufferCount];
  ComPtr<ID3D12DescriptorHeap>      uavHeap;
  ComPtr<ID3D12Resource>            positionBuffer;
  ComPtr<ID3D12CommandQueue>        computeCommandQueue;
  ComPtr<ID3D12CommandAllocator>    computeCommandAllocator;
  ComPtr<ID3D12GraphicsCommandList> computeCommandList;
  ComPtr<ID3D12PipelineState>       computePipelineState;
  ComPtr<ID3D12RootSignature>       computeRootSignature;
  ComPtr<ID3D12Resource>            vertexBuffer;

  UINT   rtvDescriptorSize;
  UINT64 fenceValue[frameBufferCount];
  HANDLE fenceEvent;

  D3D12_VIEWPORT           viewport;
  D3D12_RECT               scissorRect;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

  size_t vBufferSize;

  std::vector<VertexPos> indices;
  std::vector<Vertex>    vertices;

  std::unique_ptr<Camera>       camera;
  std::unique_ptr<ImGuiAdapter> imguiAdapter;

  friend class Window;
};

//#endif // D3D12_RENDERER_H