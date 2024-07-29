#include "Buffers.h"
#include <glm/glm.hpp>
#include <stdexcept>

bool Window::InitializeVertexBuffer(const std::vector<Vertex>& vertices)
{
  if (vertices.empty())
  {
    throw std::runtime_error("Vertex data is empty.");
  }

  vBufferSize         = vertices.size() * sizeof(Vertex);
  auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  auto resourceDesc   = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

  ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                IID_PPV_ARGS(&vertexBuffer)));
  vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

  auto            heapPropertiesUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  ID3D12Resource* vBufferUploadHeap;
  ThrowIfFailed(device->CreateCommittedResource(&heapPropertiesUpload, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                IID_PPV_ARGS(&vBufferUploadHeap)));
  vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

  D3D12_SUBRESOURCE_DATA vertexData = {};
  vertexData.pData                  = vertices.data();
  vertexData.RowPitch               = vBufferSize;
  vertexData.SlicePitch             = vBufferSize;

  UpdateSubresources(commandList.Get(), vertexBuffer, vBufferUploadHeap, 0, 0, 1, &vertexData);

  auto resBarrierVertexBuffer = CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST,
                                                                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
  commandList->ResourceBarrier(1, &resBarrierVertexBuffer);

  commandList->Close();
  ID3D12CommandList* ppCommandLists[] = {commandList.Get()};
  commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  fenceValue[frameIndex]++;
  ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));

  vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
  vertexBufferView.StrideInBytes  = sizeof(Vertex);
  vertexBufferView.SizeInBytes    = vBufferSize;

  return true;
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

void Window::UpdateConstantBuffer(const glm::mat4& rotationMat)
{
  if (!constantBuffer[frameIndex])
  {
    throw std::runtime_error("Constant buffer is not initialized.");
  }
  ConstantBuffer* cbDataBegin = nullptr;
  ThrowIfFailed(constantBuffer[frameIndex]->Map(0, nullptr, reinterpret_cast<void**>(&cbDataBegin)));
  cbDataBegin->rotationMat = rotationMat;
  constantBuffer[frameIndex]->Unmap(0, nullptr);
}

void Window::InitializeComputeBuffer(const std::vector<Vertex>& vertices)
{
  std::vector<VertexPos> positions(vertices.size());
  for (size_t i = 0; i < vertices.size(); ++i)
  {
    positions[i].position = vertices[i].pos;
    positions[i].index    = static_cast<uint32_t>(i);
  }

  size_t positionBufferSize = positions.size() * sizeof(VertexPos);

  ComPtr<ID3D12Resource> positionUploadBuffer;
  D3D12_HEAP_PROPERTIES  uploadHeapProps  = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
  D3D12_RESOURCE_DESC    uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(positionBufferSize);

  ThrowIfFailed(device->CreateCommittedResource(&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &uploadBufferDesc,
                                                D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                IID_PPV_ARGS(&positionUploadBuffer)));

  void* pData;
  ThrowIfFailed(positionUploadBuffer->Map(0, nullptr, &pData));
  memcpy(pData, positions.data(), positionBufferSize);
  positionUploadBuffer->Unmap(0, nullptr);

  D3D12_HEAP_PROPERTIES defaultHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  D3D12_RESOURCE_DESC   bufferDesc =
      CD3DX12_RESOURCE_DESC::Buffer(positionBufferSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

  ThrowIfFailed(device->CreateCommittedResource(&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                                D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                IID_PPV_ARGS(&positionBuffer)));

  commandList->CopyBufferRegion(positionBuffer.Get(), 0, positionUploadBuffer.Get(), 0, positionBufferSize);

  CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
      positionBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
  commandList->ResourceBarrier(1, &uavBarrier);

  D3D12_DESCRIPTOR_HEAP_DESC uavHeapDesc = {};
  uavHeapDesc.NumDescriptors             = 1;
  uavHeapDesc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  uavHeapDesc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  ThrowIfFailed(device->CreateDescriptorHeap(&uavHeapDesc, IID_PPV_ARGS(&uavHeap)));

  D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
  uavDesc.Buffer.NumElements               = static_cast<UINT>(positions.size());
  uavDesc.Buffer.StructureByteStride       = sizeof(VertexPos);

  CD3DX12_CPU_DESCRIPTOR_HANDLE uavHandle(uavHeap->GetCPUDescriptorHandleForHeapStart());
  device->CreateUnorderedAccessView(positionBuffer.Get(), nullptr, &uavDesc, uavHandle);
}