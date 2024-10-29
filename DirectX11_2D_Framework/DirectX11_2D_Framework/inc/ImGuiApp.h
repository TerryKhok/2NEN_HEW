#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

//imgui
#include "../../DirectX11_2D_Framework/imgui/imgui.h"
#include "../../DirectX11_2D_Framework/imgui/imgui_impl_dx11.h"
#include "../../DirectX11_2D_Framework/imgui/imgui_impl_win32.h"


class ImGuiApp
{
	friend class Window;

private:
	static void Init(HWND hWnd);

	static void Begin();

	static void Draw();

	static void Rend();

	static void Uninit();
private:
	static int fpsCounter;
};

