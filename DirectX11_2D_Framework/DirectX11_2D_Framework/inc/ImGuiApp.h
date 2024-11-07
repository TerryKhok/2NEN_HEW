#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

//imgui
#include "../../DirectX11_2D_Framework/imgui/imgui.h"
#include "../../DirectX11_2D_Framework/imgui/imgui_impl_dx11.h"
#include "../../DirectX11_2D_Framework/imgui/imgui_impl_win32.h"


#define MAGIC_ENUM_RANGE_MIN 0  // 最小値に合わせて調整
#define MAGIC_ENUM_RANGE_MAX 0x00000800   // 最大値に合わせて調整

#include "../../DirectX11_2D_Framework/magic_enum/magic_enum.hpp"

#define IMGUI_WINDOW_NAME "imGui Window"

#define IMGUI_WINDOW_WIDTH (480)
#define IMGUI_WINDOW_HEIGHT (720)

class ImGuiApp
{
	friend class Window;


	enum WINDOW_TYPE
	{
		OPTIONS,
		INSPECTER,
		TYPE_MAX
	};

private:
	ImGuiApp() = delete;

	static HRESULT Init(HINSTANCE hInstance);

	static void Draw();

	static void DrawOptionGui();
	static void DrawInspecterGui();

	static void Uninit();
private:
	static int worldFpsCounter;
	static int updateFpsCounter;

	//window関連
private:
	//コールバック関数
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	static HWND m_hWnd[TYPE_MAX];
	static ImGuiContext* context[TYPE_MAX];
	static std::unordered_map<HWND, ImGuiContext*> m_hWndContexts;
	static ComPtr<IDXGISwapChain> m_pSwapChain[TYPE_MAX];
	static ComPtr<ID3D11RenderTargetView> m_pRenderTargetView[TYPE_MAX];
	static ComPtr<ID3D11ShaderResourceView> m_pIconTexture;
	static void(*pDrawImGui[TYPE_MAX])();
};

