#pragma once

// マクロ定義
#define WINDOW_NAME  "窓 -ウィンドウ-"// ウィンドウの名前

//フレームレート
//================================================================
#define UPDATE_FPS 60
//================================================================

//================================================================
#define WORLD_FPS 120
//================================================================

//初期画面の大きさ
//================================================================
#define SCREEN_WIDTH (1152)	// ウインドウの幅
#define SCREEN_HEIGHT (864)	// ウインドウの高さ
//#define SCREEN_WIDTH (960)	// ウインドウの幅
//#define SCREEN_HEIGHT (720)	// ウインドウの高さ
//================================================================

//初期画面の大きさ
//================================================================
#define SUB_SCREEN_WIDTH (320)	// ウインドウの幅
#define SUB_SCREEN_HEIGHT (320)	// ウインドウの高さ
//================================================================

//画面を映す大きさ（これが座標系の大きさになる）
//================================================================
#define PROJECTION_WIDTH (320)
#define PROJECTION_HEIGHT (240)
//================================================================

//画面に対する物体の大きさ
//================================================================
constexpr float PROJECTION_WINDOW_WIDTH = SCREEN_WIDTH / 3.0f;
constexpr float PROJECTION_WINDOW_HEIGHT = SCREEN_HEIGHT / 3.0f;
//================================================================

//画面の大きさと座標の大きさの比
constexpr float DISPALY_ASPECT_WIDTH = static_cast<float>(SCREEN_WIDTH) / PROJECTION_WIDTH;
constexpr float DISPALY_ASPECT_HEIGHT = static_cast<float>(SCREEN_HEIGHT) / PROJECTION_HEIGHT;

//画面の大きさと座標の大きさの比
constexpr float PROJECTION_ASPECT_WIDTH = static_cast<float>(PROJECTION_WIDTH) / PROJECTION_WINDOW_WIDTH;
constexpr float PROJECTION_ASPECT_HEIGHT = static_cast<float>(PROJECTION_WIDTH) / PROJECTION_WINDOW_WIDTH;

//メインウィンドウを動かせないようにする
//================================================================
#define MAINWINDOW_LOCK
//================================================================


#ifdef DEBUG_TRUE
//常にメインの上にサブが来る
#define SUBWINDOW_IS_TOP
#endif

//ウィンドウをマウスで動かせなくする
//================================================================
#define SUBWINDOW_MOVELOCK
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

#ifdef DEBUG_TRUE
#define WM_PAUSE_DEBUG (WM_USER + 6)
#define WM_RESUME_DEBUG (WM_USER + 7)
#endif

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

#ifdef MAINLOOP__MALUTITHREAD
	//メイン処理の終わりフラグ
	static std::atomic<bool> mainLoopRun;
#endif

	//非同期ロードの終わりフラグ
	static std::atomic<bool> terminateFlag;

	//ウィンドウのハンドルに対応したオブジェクトの名前
	static std::unordered_map<HWND, const std::string&> m_hwndObjNames;
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
	static const HWND& GetMainHWnd();
	//ゲームを一時停止する
	static void PauseGame();
	//ゲームを再開する
	static void ResumeGame();
	//ゲーム停止中かどうか
	static const bool IsPause();
#ifdef DEBUG_TRUE
	static const bool IsPauseDebug();
#endif
public:
	static thread_local HWND(*pWindowSubCreate)(const std::string&, std::string, int, int, Vector2);
private:	
	static HWND WindowSubCreate(const std::string& _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//サブウィンドウ作成
	static HWND WindowSubCreateAsync(const std::string& _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//コールバック関数
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//サブウィンドウ用コールバック関数
	static LRESULT CALLBACK WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//ウィンドウを動かせるかを設定する
	static void SetWindowMovable(HWND hwnd, bool movable);
};
