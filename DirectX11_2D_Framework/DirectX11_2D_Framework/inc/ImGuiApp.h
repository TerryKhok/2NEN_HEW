#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

//imgui
#include "../../DirectX11_2D_Framework/imgui/imgui.h"
#include "../../DirectX11_2D_Framework/imgui/imgui_impl_dx11.h"
#include "../../DirectX11_2D_Framework/imgui/imgui_impl_win32.h"

#define MAGIC_ENUM_RANGE_MIN 0  // 最小値に合わせて調整
#define MAGIC_ENUM_RANGE_MAX 0x00000800   // 最大値に合わせて調整

#include "../../DirectX11_2D_Framework/magic_enum/magic_enum.hpp"

#ifdef DEBUG_TRUE

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

	enum UPDATE_VALUE_TYPE
	{
		BOX2D_POSITION,
		BOX2D_ROTATION,
		WINDOW_POSITION,
		VALUE_TYPE_MAX
	};

private:
	ImGuiApp() = delete;

	static HRESULT Init(HINSTANCE hInstance);

	static void Draw();

	static void DrawOptionGui();
	static void DrawInspectorGui();
	
	static void SetSelectedObject(GameObject* _object);

	static void RewindChange();

	static bool UpdateHandleUI(Vector2 _targetPos);
	static void DrawHandleUI(const Vector2& _targetPos);
	
	static void UnInit();
public:
	static void InvalidSelectedObject();
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
	static ImTextureID m_imIconTexture;
	static void(*pDrawImGui[TYPE_MAX])();
	static std::stack<std::function<void()>> changes;
	static std::unordered_map<std::string, std::array<int, VALUE_TYPE_MAX>> updateValues;
	static std::stack<std::unique_ptr<GameObject, void(*)(GameObject*)>> willDeleteObjects;
private:
	class HandleUI
	{
		friend class Window;

		enum HANDLE_MODE
		{
			POSITION,
			ROTATION,
			SCALE,
			HANDLE_MODE_MAX
		};

		enum MOVE_MODE
		{
			NONE,
			VERTICAL_POS,
			VERTICAL_POS_ON_MOUSE,
			HORIZON_POS,
			HORIZON_POS_ON_MOUSE,
			MOVE_POS,
			MOVE_POS_ON_MOUSE,
			MOVE_ROTATION,
			MOVE_ROTATION_ON_MOUSE,
			MOVE_SCALE,
			MOVE_SCALE_ON_MOUSE,
			MODE_MODE_MAX
		};

		friend class ImGuiApp;

		HandleUI() = default;
		HRESULT Init();
		bool Update(Vector2 _targetPos);
		void Draw(const GameObject* _target,const Vector2 _targetPos);
		std::function<void()> SetChangeValue(GameObject* _object, MOVE_MODE _moveMode);

		static ComPtr<ID3D11Buffer> m_arrowVertexBuffer;
		static ComPtr<ID3D11Buffer> m_arrowIndexBuffer;
		HANDLE_MODE handleMode = POSITION;
		MOVE_MODE moveMode = NONE;
	};

	static HandleUI handleUi;
};


#endif