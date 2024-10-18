#pragma once

// マクロ定義
#define CLASS_NAME   "DX11"// ウインドウクラスの名前
#define WINDOW_NAME  "DX2DFramework"// ウィンドウの名前

//フレームレート
//==============================================================
#define FPS 60
//==============================================================

//初期画面の大きさ
//================================================================
#define SCREEN_WIDTH (640)	// ウインドウの幅
#define SCREEN_HEIGHT (480)	// ウインドウの高さ
//================================================================

//画面を映す大きさ（これが座標系の大きさになる）
//================================================================
#define PROJECTION_WIDTH (640)
#define PROJECTION_HEIGHT (480)
//================================================================

//Fpsを表示するかどうか（たぶんあとで動的に変更できるようにする）
//================================================================
#define SHOW_FPS
//================================================================


class Window final
{
	Window() = delete;

	static HWND hWnd;
	static MSG msg;
	static RECT windowRect;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long oldCount;
	static int fpsCounter;		//FPS計測変数
	static long long oldTick;	//前回計測時
	static long long nowTick;	//今回計測時
	static long long nowCount;

	//スレッドの終わりフラグ
	static std::atomic<bool> terminateFlag;
public:
	//ウィンドウ生成
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//ウィンドウ初期化(シーンの初期化を渡す)
	static LRESULT WindowInit(void(*p_mainInitFunc)(void));
	//ウィンドウ更新
	static LRESULT WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/);
	//非同期更新
	static LRESULT WindowUpdate(std::future<void>& sceneFuture,bool& loading);
	//ウィンドウかたずけ
	static int WindowEnd(HINSTANCE hInstance);
private:	
	//コールバック関数
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
