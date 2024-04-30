#pragma once

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

int initImgui(ID3D12Device* device, int frameBufferCount, HWND hWnd);

int startMainImgui();

int endMainImgui();

int killImgui();
