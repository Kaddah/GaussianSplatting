#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

void initImgui(ID3D12Device* device, int frameBufferCount, HWND hWnd);

void startMainImgui();

void endMainImgui(ID3D12GraphicsCommandList* commandList);

void killImgui();
