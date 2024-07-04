#include "ImguiAdapter.h"
#include <DxException.h>
#include <glm/glm.hpp>

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
void ImGuiAdapter::createWindow(float& alphaX, float& alphaY, float& alphaZ, float& cameraSpeed, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp)
{
  ImGui::Begin("Gaussian Splatting");
  ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
  ImGui::SliderFloat("Camera Speed", &cameraSpeed, 1.0f, 10.0f);
  if (ImGui::Button("Reset Camera"))
  {
    cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  }
  ImGui::Spacing();
  ImGui::Text("Object Position: (%.2f, %.2f, %.2f)", alphaX, alphaY, alphaZ);
  if (ImGui::Button("Reset Object"))
  {
    alphaX = 0.0f;
    alphaY = 0.0f;
    alphaZ = 0.0f;
  }  
  ImGui::End();
}
