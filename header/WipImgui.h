#pragma once

#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class ImGuiAdapter
{
public:
  ImGuiAdapter(const ComPtr<ID3D12Device>& device, int frameBufferCount, HWND hWnd);
  ~ImGuiAdapter();

  void startMainImGui();
  void renderImGui();
  void commandList(const ComPtr<ID3D12GraphicsCommandList>& commandList);

private:
  ComPtr<ID3D12DescriptorHeap> imguiSRVDescriptorHeap;
};
