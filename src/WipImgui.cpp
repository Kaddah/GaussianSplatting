#include "WipImgui.h"
#include <DxException.h>

ComPtr<ID3D12DescriptorHeap> createSRVDescriptorHeap(const ComPtr<ID3D12Device>& device)
{
  ComPtr<ID3D12DescriptorHeap> srvHeap;
  D3D12_DESCRIPTOR_HEAP_DESC   srvHeapDesc = {};
  srvHeapDesc.NumDescriptors               = 1;
  srvHeapDesc.Type                         = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
  srvHeapDesc.Flags                        = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
  srvHeapDesc.NodeMask                     = 0;
  ThrowIfFailed(device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvHeap)));
  return srvHeap;
}

ImGuiAdapter::ImGuiAdapter(const ComPtr<ID3D12Device>& device, int frameBufferCount, HWND hWnd)
    : imguiSRVDescriptorHeap(createSRVDescriptorHeap(device))
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplWin32_Init(hWnd);
  ImGui_ImplDX12_Init(device.Get(), frameBufferCount, DXGI_FORMAT_R8G8B8A8_UNORM, imguiSRVDescriptorHeap.Get(),
                      imguiSRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
                      imguiSRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}
ImGuiAdapter::~ImGuiAdapter()
{
  ImGui_ImplDX12_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();
}
void ImGuiAdapter::startMainImGui()
{
  ImGui_ImplDX12_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();  
}
void ImGuiAdapter::renderImGui()
{
  ImGui::Render();
}
void ImGuiAdapter::commandList(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
  commandList->SetDescriptorHeaps(1, imguiSRVDescriptorHeap.GetAddressOf());
  
  if (auto drawData = ImGui::GetDrawData())
  {
    ImGui_ImplDX12_RenderDrawData(drawData, commandList.Get());
  }
}
