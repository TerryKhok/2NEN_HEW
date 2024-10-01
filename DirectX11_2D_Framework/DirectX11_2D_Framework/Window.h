#pragma once

//フレームレート
//==============================================================
#define FPS 60
//==============================================================

// マクロ定義
#define CLASS_NAME   "DX11"// ウインドウクラスの名前
#define WINDOW_NAME  "DX2DFramework"// ウィンドウの名前

#define SCREEN_WIDTH (640)	// ウインドウの幅
#define SCREEN_HEIGHT (480)	// ウインドウの高さ

const char* relativePath(const char* fullPath);
void setConsoleTextColor(unsigned int color);

#ifdef DEBUG_TRUE

#define LOG(format, ...) printf_s("\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__)
#define LOG_ERROR(format, ...) setConsoleTextColor(4);printf_s("\n-ERROR!!-\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__);setConsoleTextColor(7)
#define LOG_WARNING(format, ...) setConsoleTextColor(6);printf_s("\n-WARNING!-\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__);setConsoleTextColor(7)

#define LOG_NL printf_s("\n")

#else
#define LOG(format, ...)  
#define LOG_ERROR(format, ...)  
#define LOG_WARNING(format, ...)  

#define LOG_NL
#endif

class Window final
{
	Window() = delete;

	static HWND hWnd;
	static MSG msg;
	static RECT windowRect;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long oldCount;
	static int fpsCounter;						//FPS計測変数
	static long long oldTick;	//前回計測時
	static long long nowTick;				//今回計測時
	static long long nowCount;

	//flag to signal termination
	static std::atomic<bool> terminateFlag;
public:
	//ウィンドウ生成
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//ウィンドウ初期化(シーンの初期化を渡す)
	static LRESULT WindowInit(void(*p_sceneInitFunc)(void));
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
