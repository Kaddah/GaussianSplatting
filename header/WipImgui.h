#pragma once
#ifndef IMGUI_INIT_H
#define IMGUI_INIT_H

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include <Window.h>
#include <GaussianRenderer.h>

int initImgui(ID3D12Device* device, int frameBufferCount, HWND hWnd);

int startMainImgui();

int endMainImgui();

int killImgui();

#endif // IMGUI_INIT_H