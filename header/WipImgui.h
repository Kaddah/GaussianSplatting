#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

void initImgui(ID3D12Device* device, int frameBufferCount, HWND hWnd,ID3D12DescriptorHeap* srvHeap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

void startMainImgui();

void endMainImgui(ID3D12GraphicsCommandList* commandList);	

void killImgui();
