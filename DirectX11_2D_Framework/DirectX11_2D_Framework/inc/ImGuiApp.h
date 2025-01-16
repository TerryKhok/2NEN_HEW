#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

//古いバージョンのキーマップをなくす
#define IMGUI_DISABLE_OBSOLETE_KEYIO

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

#define ANIM_CLIP_EXTENSION "amcp"
#define ANIM_CLIP_EXTENSION_DOT "." ANIM_CLIP_EXTENSION

class ImGuiApp
{
	friend class Window;
	friend class Input;

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
	static void DrawMainGui(ImGuiContext* _mainContext);
	
	static void SetSelectedObject(GameObject* _object);
	static void CloneSelectedObject();
	static void CopySelectedObject();
	static void PastSelectedObject();
	static void CutSelectedObject();
	static void DeleteSelectedObject();

	static void ClearStack();
	static bool IsSceneChange();

	static void RewindChange();

	static bool UpdateHandleUI(Vector2 _targetPos);
	static void DrawHandleUI(const Vector2& _targetPos);

	static void SaveSceneFileDialog(std::filesystem::path& _path);
	
	static void UnInit();

	static void ImGuiSetKeyMap(ImGuiContext* _imguiContext);
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
	static void(*pDrawImGui[TYPE_MAX])();
	static std::unordered_map<std::string, std::array<int, VALUE_TYPE_MAX>> updateValues;
	static bool showMainEditor;
public:
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
		void Draw(GameObject* _target,const Vector2 _targetPos);
		std::function<void()> SetChangeValue(GameObject* _object, MOVE_MODE _moveMode);
	public:
		//functionの引数であるGameObject*を使用する場合はnullptrかチェックする必要がある
		void SetUploadFile(std::string _uploadStr, std::function<void(GameObject*, std::filesystem::path)>&& _func, std::vector<std::string>&& _extensions);
		//ハンドルの機能をロックする
		void LockHandle(bool _lock,const char* _lockName);
	private:
		static ComPtr<ID3D11Buffer> m_arrowVertexBuffer;
		static ComPtr<ID3D11Buffer> m_arrowIndexBuffer;
		bool lock = false;
		std::list<std::string> lockNames;
		HANDLE_MODE handleMode = POSITION;
		MOVE_MODE moveMode = NONE;
		std::string uploadStr = {};
		std::vector<std::string> extensions;
		std::function<void(GameObject*, std::filesystem::path)> linkFunc = {};
	};
private:
	static HandleUI handleUi;
};

#else
class ImGuiApp
{
public:
	class HandleUI
	{

	};
};
#endif