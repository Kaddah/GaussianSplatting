#pragma once
#ifndef IMGUI_INIT_H
#define IMGUI_INIT_H

#include <Windows.h>
#include "imgui.h"

int initImGui(HWND hwnd);

int startMainImgui();

int endMainImgui();

int killImgui();

#endif // IMGUI_INIT_H