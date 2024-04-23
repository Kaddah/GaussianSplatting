# include <d3d12.h>
# include <imgui.h>
# include <imgui_impl_dx12.h>
# include <imgui_impl_win32.h>
#include <dxgi1_4.h>
#include <tchar.h>

//https://github.com/ocornut/imgui/wiki/Getting-Started

namespace imgui {
	void init(HWND hwnd, ID3D12Device* device, ID3D12CommandQueue* queue) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplWin32_Init(hwnd);
		ImGui::StyleColorsDark(); // mach dark mode
		//ImGui_ImplDX12_Init(device, 60, DXGI_FORMAT_R8G8B8A8_UNORM);
		//ImGui_ImplDX12_Init(YOUR_D3D_DEVICE, NUM_FRAME_IN_FLIGHT, YOUR_RENDER_TARGET_DXGI_FORMAT,YOUR_SRV_DESC_HEAP,
		}

	void gibDemoWindow() {
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		}

	void clearDemoWindow() {
		ImGui::Render();
		//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), what);
		}

	void shutdown() {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}	
}