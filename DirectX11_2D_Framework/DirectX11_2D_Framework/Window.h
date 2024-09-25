#pragma once

// マクロ定義
#define CLASS_NAME   "DX11"// ウインドウクラスの名前
#define WINDOW_NAME  "DX2DFramework"// ウィンドウの名前

#define SCREEN_WIDTH (640)	// ウインドウの幅
#define SCREEN_HEIGHT (480)	// ウインドウの高さ

const char* relativePath(const char* fullPath);

#ifdef DEBUG_TRUE
#define LOG(format, ...) printf_s("\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__)

#else
#define LOG(format, ...)  

#endif

class Window
{
	Window() = delete;

	static HWND hWnd;
	static MSG msg;
	static RECT windowRect;
public:
	//ウィンドウ生成
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//メインループ(更新したい処理を引数に渡す)
	static LRESULT MainLoop(void(*p_initFunc)(void), void(*p_updateFunc)(void), void(*p_drawFunc)(void), int fps);
	//ウィンドウかたずけ
	static int WindowEnd(HINSTANCE hInstance);
private:
	//コールバック関数
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
