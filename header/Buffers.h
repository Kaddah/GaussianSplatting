cpp Code kopieren
#pragma once

#include "Vertex.h"
#include <d3d12.h>
#include <glm/glm.hpp>
#include <vector>
#include <wrl/client.h>

    using Microsoft::WRL::ComPtr;

class Window;

class Buffers
{
public:
  Buffers();
  ~Buffers();

  bool InitializeVertexBuffer(Window* window, const std::vector<Vertex>& vertices);
  void UpdateVertexBuffer(Window* window, const std::vector<Vertex>& vertices);
  void UpdateConstantBuffer(Window* window, const glm::mat4& rotationMat);
  void InitializeComputeBuffer(Window* window, const std::vector<Vertex>& vertices);

private:
  // Buffers
  ComPtr<ID3D12Resource>   vertexBuffer;
  ComPtr<ID3D12Resource>   constantBuffer[2];
  ComPtr<ID3D12Resource>   positionBuffer;
  D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
  size_t                   vBufferSize;
};