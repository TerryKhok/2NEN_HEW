#pragma once

// マクロ定義
#define WINDOW_NAME  "DX2DFramework"// ウィンドウの名前

//フレームレート
//==============================================================
#define FPS 60
//==============================================================

//初期画面の大きさ
//================================================================
#define SCREEN_WIDTH (960)	// ウインドウの幅
#define SCREEN_HEIGHT (720)	// ウインドウの高さ
//================================================================

//初期画面の大きさ
//================================================================
#define SUB_SCREEN_WIDTH (640)	// ウインドウの幅
#define SUB_SCREEN_HEIGHT (640)	// ウインドウの高さ
//================================================================

//画面を映す大きさ（これが座標系の大きさになる）
//================================================================
#define PROJECTION_WIDTH (640)
#define PROJECTION_HEIGHT (480)
//================================================================

	//画面の大きさと座標の大きさの比
constexpr float DISPALY_ASPECT_WIDTH = static_cast<float>(SCREEN_WIDTH) / PROJECTION_WIDTH;
constexpr float DISPALY_ASPECT_HEIGHT = static_cast<float>(SCREEN_HEIGHT) / PROJECTION_HEIGHT;

//Fpsを表示するかどうか（たぶんあとで動的に変更できるようにする）
//================================================================
#define SHOW_FPS
//================================================================


class Window final
{
public:
	enum HANDLE_TYPE
	{
		MAIN,
		HANDLE_MAX
	};

private:
	Window() = delete;

	static HWND mainHwnd;
	static MSG msg;
	static RECT windowSize;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long oldCount;
	static int fpsCounter;		//FPS計測変数
	static long long oldTick;	//前回計測時
	static long long nowTick;	//今回計測時
	static long long nowCount;

#ifdef MAINLOOP__MALUTITHREAD
	//メイン処理の終わりフラグ
	static std::atomic<bool> mainLoopRun;
#endif

	//非同期ロードの終わりフラグ
	static std::atomic<bool> terminateFlag;

	//ウィンドウを動かしているかのフラグ
	static std::atomic<bool> windowSizeMove;
public:
	//ウィンドウ生成
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//サブウィンドウ作成
	static LRESULT WindowSubCreate(HINSTANCE hInstance,int nCmdShow,const char* _windowName = "SUB");
	//ウィンドウ初期化(シーンの初期化を渡す)
	static LRESULT WindowInit(void(*p_mainInitFunc)(void));
	//ウィンドウ更新
	static LRESULT WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/);
	//非同期更新
	static LRESULT WindowUpdate(std::future<void>& sceneFuture, bool& loading);
	//ウィンドウかたずけ
	static int WindowEnd(HINSTANCE hInstance);
	//メインのウィンドウハンドル取得
	static const HWND& GetMainHwnd(){return mainHwnd;}
private:	
	//コールバック関数
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//サブウィンドウ用コールバック関数
	static LRESULT CALLBACK WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
