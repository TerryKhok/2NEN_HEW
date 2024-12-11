#pragma once

// マクロ定義
#define WINDOW_NAME  "DX2DFramework"// ウィンドウの名前

//フレームレート
//================================================================
#define UPDATE_FPS 60
//================================================================

//================================================================
#define WORLD_FPS 240
//================================================================

//初期画面の大きさ
//================================================================
#define SCREEN_WIDTH (960)	// ウインドウの幅
#define SCREEN_HEIGHT (720)	// ウインドウの高さ
//================================================================

//初期画面の大きさ
//================================================================
#define SUB_SCREEN_WIDTH (320)	// ウインドウの幅
#define SUB_SCREEN_HEIGHT (320)	// ウインドウの高さ
//================================================================

//画面を映す大きさ（これが座標系の大きさになる）
//================================================================
#define PROJECTION_WIDTH (640)
#define PROJECTION_HEIGHT (480)
//================================================================

//画面の大きさと座標の大きさの比
constexpr float DISPALY_ASPECT_WIDTH = static_cast<float>(SCREEN_WIDTH) / PROJECTION_WIDTH;
constexpr float DISPALY_ASPECT_HEIGHT = static_cast<float>(SCREEN_HEIGHT) / PROJECTION_HEIGHT;

//メインウィンドウを動かせないようにする
//================================================================
#define MAINWINDOW_LOCK
//================================================================

//常にメインの上にサブが来る
//================================================================
#define SUBWINDOW_IS_TOP
//================================================================

//ウィンドウの位置にカメラを置く
//================================================================
#define CAMERA_ON_WINDOW
//================================================================


#define WM_CREATE_NEW_WINDOW (WM_USER + 1)
#define WM_DELETE_WINDOW (WM_USER + 2)
#define WM_ADJUST_Z_ORDER (WM_USER + 3)
#define WM_PAUSE_GAME (WM_USER + 4)
#define WM_RESUME_GAME (WM_USER + 5)

//ウィンドウの位置を取得する
Vector2 GetWindowPosition(HWND _hWnd);
//ウィンドウの位置を変更する
void SetWindowPosition(HWND _hWnd, Vector2 pos);

class Window final
{
public:
	//モニターの解像度所得
	static int MONITER_HALF_WIDTH;
	static int MONITER_HALF_HEIGHT;

private:
	Window() = delete;

	static HINSTANCE m_hInstance;
	static int m_nCmdShow;
	static HWND mainHwnd;
	static MSG msg;
	static RECT windowSize;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long worldOldCount;
	static long long updateOldCount;
	static long long worldLag;
	static long long updateLag;
	static int worldFpsounter;	//FPS計測変数
	static int updateFpsCounter;		//FPS計測変数
	static long long oldTick;	//前回計測時
	static long long nowTick;	//今回計測時
	static long long nowCount;

#ifdef MAINLOOP__MALUTITHREAD
	//メイン処理の終わりフラグ
	static std::atomic<bool> mainLoopRun;
#endif

	//非同期ロードの終わりフラグ
	static std::atomic<bool> terminateFlag;

	//ウィンドウのハンドルに対応したオブジェクトの名前
	static std::unordered_map<HWND, std::string> m_hwndObjNames;
public:
	//ウィンドウ生成
	static LRESULT WindowMainCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

	//サブウィンドウ削除
	static void WindowSubRelease(HWND hWnd);
	//シーンローディング開始
	static void WindowSubLoadingBegin();
	//シーンローディング終了(すべてのウィンドウを表示する)
	static void WindowSubLoadingEnd();
	//現在のウィンドウを保持しているオブジェクトを破棄する
	static void WindowSubHide();
	static void WindowSubHide(HWND _hWnd);

	//ウィンドウ初期化(シーンの初期化を渡す)
	static LRESULT WindowInit(void(*p_mainInitFunc)(void));
	//ウィンドウ更新
	static LRESULT WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/);
	//非同期更新
	static LRESULT WindowUpdate(std::future<void>& sceneFuture, bool& loading);
	//ウィンドウかたずけ
	static int WindowEnd();
	//メインのウィンドウハンドル取得
	static const HWND& GetMainHwnd(){return mainHwnd;}
public:
	static thread_local HWND(*pWindowSubCreate)(std::string, std::string, int, int, Vector2);
private:	
	static HWND WindowSubCreate(std::string _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//サブウィンドウ作成
	static HWND WindowSubCreateAsync(std::string _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//コールバック関数
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//サブウィンドウ用コールバック関数
	static LRESULT CALLBACK WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//ウィンドウを動かせるかを設定する
	static void SetWindowMovable(HWND hwnd, bool movable);
};
