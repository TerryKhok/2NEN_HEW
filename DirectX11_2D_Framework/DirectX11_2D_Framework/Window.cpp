#include "Window.h"
#include "HelloBox2d.h"


void CreateConsoleWindow() {
	// Allocates a new console for the calling process
	AllocConsole();

	// Redirect standard input, output, and error to the console window
	freopen_s((FILE**)stdin,  "CONIN$",  "r", stdin);
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);

	// Optional: Set console title
	SetConsoleTitle("Debug Console");

	std::cout << "Debugging Console Initialized!" << std::endl;
}

void CloseConsoleWindow() {
	FreeConsole();
}

HWND  Window::mainHwnd;
MSG Window::msg;
RECT Window::windowSize;
LARGE_INTEGER Window::liWork;
long long Window::frequency;
long long Window::oldCount;
int Window::fpsCounter = 0;						//FPS計測変数
long long Window::oldTick = GetTickCount64();	//前回計測時
long long Window::nowTick = oldTick;				//今回計測時
long long Window::nowCount = oldCount;

std::atomic<bool> Window::terminateFlag(false);

//ウィンドウを動かしているかのフラグ
std::atomic<bool> Window::windowSizeMove(false);

LRESULT Window::WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

#ifdef DEBUG_TRUE
	//コンソール画面起動
	CreateConsoleWindow();

	//メモリリーク検知
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// ウィンドウクラス情報をまとめる
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MAIN_WINDOW";
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	wc.lpfnWndProc = WndProcSub;
	wc.lpszClassName = "SUB_WINDOW";

	RegisterClassEx(&wc);

	// ウィンドウの情報をまとめる
	mainHwnd = CreateWindowEx(
		0,										// 拡張ウィンドウスタイル
		"MAIN_WINDOW",								// ウィンドウクラスの名前
		WINDOW_NAME,									// ウィンドウの名前
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,// ウィンドウスタイル
		CW_USEDEFAULT,							// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,							// ウィンドウの左上Ｙ座標 
		SCREEN_WIDTH,							// ウィンドウの幅
		SCREEN_HEIGHT,							// ウィンドウの高さ
		NULL,									// 親ウィンドウのハンドル
		NULL,									// メニューハンドルまたは子ウィンドウID
		hInstance,								// インスタンスハンドル
		NULL);									// ウィンドウ作成データ

	//ウィンドウのサイズを修正
	RECT rc1, rc2;
	GetWindowRect(mainHwnd, &rc1);
	GetClientRect(mainHwnd, &rc2);
	int sx = SCREEN_WIDTH;
	int sy = SCREEN_HEIGHT;
	sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
	sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
	SetWindowPos(mainHwnd, NULL, 0, 0, sx, sy, (SWP_NOZORDER |
		SWP_NOOWNERZORDER | SWP_NOMOVE));

	// 指定されたウィンドウの表示状態を設定(ウィンドウを表示)
	ShowWindow(mainHwnd, nCmdShow);
	// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
	UpdateWindow(mainHwnd);

	//FPS固定用変数
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	oldCount = liWork.QuadPart;

	//DirectX生成
	DirectX11::D3D_Create(mainHwnd);



	return LRESULT();
}

LRESULT Window::WindowSubCreate(HINSTANCE hInstance, int nCmdShow, const char* _windowName)
{

	HWND hWnd = CreateWindowEx(
		0,										// 拡張ウィンドウスタイル
		"SUB_WINDOW",								// ウィンドウクラスの名前
		_windowName,							// ウィンドウの名前
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,// ウィンドウスタイル
		CW_USEDEFAULT,							// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,							// ウィンドウの左上Ｙ座標 
		SUB_SCREEN_WIDTH,							// ウィンドウの幅
		SUB_SCREEN_HEIGHT,							// ウィンドウの高さ
		NULL,									// 親ウィンドウのハンドル
		NULL,									// メニューハンドルまたは子ウィンドウID
		hInstance,								// インスタンスハンドル
		NULL								// ウィンドウ作成データ
	);

	//ウィンドウのサイズを修正
	RECT rc1, rc2;
	GetWindowRect(hWnd, &rc1);
	GetClientRect(hWnd, &rc2);
	int sx = SUB_SCREEN_WIDTH;
	int sy = SUB_SCREEN_HEIGHT;
	sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
	sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
	SetWindowPos(hWnd, NULL, 0, 0, sx, sy, (SWP_NOZORDER |
		SWP_NOOWNERZORDER | SWP_NOMOVE));

	// 指定されたウィンドウの表示状態を設定(ウィンドウを表示)
	ShowWindow(hWnd, nCmdShow);
	// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
	UpdateWindow(hWnd);

	//リストに追加
	//hWnds.push_back(hWnd);

	DirectX11::CreateWindowSwapchain(hWnd);

	return LRESULT();
}

LRESULT Window::WindowInit(void(*p_mainInitFunc)(void))
{
	//初期化処理
	//=================================================

	SceneManager::m_sceneList.clear();
	//Box2Dワールド作成
	Box2D::WorldManager::CreateWorld();

	RenderManager::Init();
	p_mainInitFunc();

	SceneManager::Init();

#ifdef DEBUG_TRUE
	Box2DBodyManager::Init();
#endif 

#ifdef BOX2D_UPDATE_MULTITHREAD
	//ワールドの更新スタート
	Box2D::WorldManager::StartWorldUpdate();
#endif

#ifdef SFTEXT_TRUE
	//テキスト初期化(Font生成)
	SFTextManager::Init();
#endif
	//=================================================

	return LRESULT();
}


LRESULT Window::WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/)
{
	const long long frameCount = frequency / FPS;
	// ゲームループ
	while (1)
	{
		// 新たにメッセージがあれば
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			// ウィンドウプロシージャにメッセージを送る
			DispatchMessage(&msg);

			// 「WM_QUIT」メッセージを受け取ったらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}

			continue;
		}

		
		//現在時間を取得
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		if (nowCount >= oldCount + frameCount)
		{
			//Animator用のカウント更新
			AnimatorManager::deltaCount = nowCount - oldCount;

#ifndef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::WorldUpdate();
#endif
			Input::Get().Update();

#ifdef DEBUG_TRUE
			try
			{
				SceneManager::m_currentScene->Update();
			}
			//例外キャッチ(nullptr参照とか)
			catch (const std::exception& e) {
				LOG_ERROR(e.what());
			}
#else
			SceneManager::m_currentScene->Update();
#endif
			ObjectManager::UpdateObjectComponent();

			Box2DBodyManager::ExcuteMoveFunction();

			//カメラ関連行列セット
			CameraManager::SetCameraMatrix();

			DirectX11::D3D_StartRender();

			RenderManager::Draw();

#ifdef SFTEXT_TRUE
			SFTextManager::ExcuteDrawString();
#endif

			DirectX11::D3D_FinishRender();

			oldCount = nowCount;
			fpsCounter++;
			nowTick = GetTickCount64();

			if (nowTick >= oldTick + 1000)
			{
#ifdef SHOW_FPS
				char str[32];
				wsprintfA(str, "FPS = %d", fpsCounter);
				SetWindowTextA(mainHwnd, str);
#endif

				fpsCounter = 0;
				oldTick = nowTick;
			}
		}
	}
	

	return LRESULT();
}

LRESULT Window::WindowUpdate(std::future<void>& sceneFuture,bool& loading)
{
	const long long frameCount = frequency / FPS;

	// Example main loop with a loading screen
	bool loadingComplete = false;

	// ゲームループ
	while (!loadingComplete && !terminateFlag)
	{
		// 新たにメッセージがあれば
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// ウィンドウプロシージャにメッセージを送る
			DispatchMessage(&msg);

			// 「WM_QUIT」メッセージを受け取ったらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
		}
		else
		{
			//現在時間を取得
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart;
			if (nowCount >= oldCount + frameCount)
			{
				//Animator用のカウント更新
				AnimatorManager::deltaCount = nowCount - oldCount;

#ifndef BOX2D_UPDATE_MULTITHREAD
				Box2D::WorldManager::WorldUpdate();
#endif
				Input::Get().Update();

#ifdef DEBUG_TRUE
				try
				{
					SceneManager::m_currentScene->Update();
				}
				//例外キャッチ(nullptr参照とか)
				catch (const std::exception& e) {
					LOG_ERROR(e.what());
				}
#else
				SceneManager::m_currentScene->Update();
#endif

				ObjectManager::UpdateObjectComponent();

				Box2DBodyManager::ExcuteMoveFunction();

				CameraManager::SetCameraMatrix();

				DirectX11::D3D_StartRender();

				RenderManager::Draw();

#ifdef SFTEXT_TRUE
				SFTextManager::ExcuteDrawString();
#endif

				DirectX11::D3D_FinishRender();

				oldCount = nowCount;
				fpsCounter++;
				nowTick = GetTickCount64();

				if (nowTick >= oldTick + 1000)
				{
#ifdef SHOW_FPS
					char str[32];
					wsprintfA(str, "FPS = %d", fpsCounter);
					SetWindowTextA(mainHwnd, str);
#endif

					fpsCounter = 0;
					oldTick = nowTick;
				}

				// Check if the loading is complete
				if (sceneFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
					loadingComplete = true;
				}
			}
		}
	}

	// Wait for async task to finish
	if (!loadingComplete) {
		//テクスチャ読み込みなしに設定する
		TextureAssets::LoadEnd();

		LOG("Scene loading stopped!,Please wait for end process");
		sceneFuture.get();
	}
	else
	{
		LOG("Loading Complete!");
		loading = false;
	}

	return LRESULT();
}

int Window::WindowEnd(HINSTANCE hInstance)
{

#ifdef BOX2D_UPDATE_MULTITHREAD
	Box2D::WorldManager::StopWorldUpdate();
#endif 
	//オブジェクトの破棄
	ObjectManager::Uninit();

	//オブジェクトを削除する(念入りに)
	ObjectManager::CleanAllObjectList();
	
	//すべてのワールドを削除する
	Box2D::WorldManager::DeleteAllWorld();

	//シーンの破棄
	SceneManager::Uninit();

	//DirectXかたずけ
	DirectX11::D3D_Release();

#ifdef DEBUG_TRUE
	//コンソール画面閉じる
	CloseConsoleWindow();
#endif

	UnregisterClass("MAIN_WINDOW", hInstance);

	UnregisterClass("SUB_WINDOW", hInstance);

	//メモリリーク詳細検知
	//(unique_ptr系はこのあと解放されるので検知されてしまう)
	//_CrtDumpMemoryLeaks();

	return (int)msg.wParam;
}

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:// ウィンドウ破棄のメッセージ
		PostQuitMessage(0);// 「WM_QUIT」メッセージを送る　→　アプリ終了
		break;

	case WM_CLOSE:  // 「x」ボタンが押されたら
	{
		int res = MessageBoxA(NULL, "終了しますか？", "確認", MB_OKCANCEL);
		if (res == IDOK) {
			terminateFlag = true;
			DestroyWindow(hWnd);  // 「WM_DESTROY」メッセージを送る
		}
	}
	break;

	case WM_MOVING:
	{
		//カメラ関連行列セット
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::ExcuteDrawString();
#endif

		DirectX11::D3D_FinishRender();
	}
	break;

	case WM_KEYDOWN: //キー入力があったメッセージ
		if (LOWORD(wParam) == VK_ESCAPE) { //入力されたキーがESCAPEなら
			PostMessage(hWnd, WM_CLOSE, wParam, lParam);//「WM_CLOSE」を送る
		}
		break;

	case WM_MOUSEMOVE:
	{
		GetCursorPos(&Input::mousePoint);
		ScreenToClient(hWnd, &Input::mousePoint);
		static const LONG HALF_SCREEN_WIDTH = SCREEN_WIDTH / 2;
		static const LONG HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;
		Input::mousePoint.x = Input::mousePoint.x * SCREEN_WIDTH / windowSize.right - HALF_SCREEN_WIDTH;
		Input::mousePoint.y = Input::mousePoint.y * SCREEN_WIDTH / windowSize.bottom - HALF_SCREEN_HEIGHT;
		Input::mousePoint.y *= (LONG)-1.0;
	}
	break;

	case WM_LBUTTONDOWN:
		Input::isMouseLeft = true;
		Input::isMouseDownLeft = true;
		break;
	case WM_LBUTTONUP:
		Input::isMouseLeft = false;
		Input::isMouseUpLeft = true;
		break;

	case WM_RBUTTONDOWN:
		Input::isMouseRight = true;
		Input::isMouseDownRight = true;
		break;
	case WM_RBUTTONUP:
		Input::isMouseRight = false;
		Input::isMouseUpRight = true;
		break;

	case WM_MOUSEWHEEL:
		Input::mouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		break;

		//画面の大きさ取得
	case WM_SIZE:
		GetClientRect(hWnd, &windowSize);
		break;
	default:
		// 受け取ったメッセージに対してデフォルトの処理を実行
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

LRESULT Window::WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:  // 「x」ボタンが押されたら
	{
		auto& swaplist = DirectX11::m_pSwapChainList;
		auto swapiter = swaplist.find(hWnd);
		if (swapiter != swaplist.end())
		{
			swaplist.erase(swapiter);
		}

		auto& viewlist = DirectX11::m_pRenderTargetViewList;
		auto viewiter = viewlist.find(hWnd);
		if (viewiter != viewlist.end())
		{
			viewlist.erase(viewiter);
		}

		DestroyWindow(hWnd);
	}
	break;

	case WM_MOVING:
	{		
		//カメラ関連行列セット
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::ExcuteDrawString();
#endif

		DirectX11::D3D_FinishRender();
	}
	break;

	default:
		// 受け取ったメッセージに対してデフォルトの処理を実行
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return LRESULT();
}


