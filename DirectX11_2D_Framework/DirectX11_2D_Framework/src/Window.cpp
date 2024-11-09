

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

#ifdef MAINLOOP__MALUTITHREAD
//メイン処理の終わりフラグ
std::atomic<bool> Window::mainLoopRun;
#endif

//モニターの解像度所得
int Window::MONITER_HALF_WIDTH = GetSystemMetrics(SM_CXSCREEN) / 2;
int Window::MONITER_HALF_HEIGHT = GetSystemMetrics(SM_CYSCREEN) / 2;


HINSTANCE Window::m_hInstance;
int Window::m_nCmdShow;
HWND  Window::mainHwnd;
MSG Window::msg;
RECT Window::windowSize;
LARGE_INTEGER Window::liWork;
long long Window::frequency;
long long Window::worldOldCount;
long long Window::updateOldCount;
long long Window::worldLag = 0;
long long Window::updateLag = 0;
int Window::worldFpsounter = 0;		//FPS計測変数
int Window::updateFpsCounter = 0;						//FPS計測変数
long long Window::oldTick = GetTickCount64();	//前回計測時
long long Window::nowTick = oldTick;				//今回計測時
long long Window::nowCount = worldOldCount;

std::atomic<bool> Window::terminateFlag(false);

thread_local HWND(*Window::pWindowSubCreate)(std::string, std::string, int, int) = WindowSubCreate;

//ウィンドウのハンドルに対応したオブジェクトの名前
std::unordered_map<HWND, std::string> Window::m_hwndObjNames;

LRESULT Window::WindowMainCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//DebugとReleaseのDpiを統一するため
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	//モニターの解像度所得
	MONITER_HALF_WIDTH = GetSystemMetrics(SM_CXSCREEN) / 2;
	MONITER_HALF_HEIGHT = GetSystemMetrics(SM_CYSCREEN) / 2;

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
		0, // Extended styles,// 拡張ウィンドウスタイル
		"MAIN_WINDOW",								// ウィンドウクラスの名前
		WINDOW_NAME,									// ウィンドウの名前
#ifdef MAINWINDOW_LOCK
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,	// ウィンドウスタイル
#else
		WS_OVERLAPPEDWINDOW,
#endif
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

	SetWindowPosition(mainHwnd, { 0.0f,0.0f });

	//FPS固定用変数
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	worldOldCount = liWork.QuadPart;
	updateOldCount = liWork.QuadPart;

	//DirectX生成
	DirectX11::D3D_Create(mainHwnd);

#ifdef DEBUG_TRUE
	ImGuiApp::Init(hInstance);
#endif

	m_hInstance = hInstance;
	m_nCmdShow = nCmdShow;

	return LRESULT();
}

std::string objectName;
std::string windowName;
int windowWidth;
int windowHeight;


HWND Window::WindowSubCreate(std::string _objName, std::string _windowName, int _width, int _height)
{
	HWND hWnd = CreateWindowEx(
		0,										// 拡張ウィンドウスタイル
		"SUB_WINDOW",								// ウィンドウクラスの名前
		_windowName.c_str(),							// ウィンドウの名前
		WS_OVERLAPPED | WS_CAPTION /*| WS_SYSMENU*/,// ウィンドウスタイル
		CW_USEDEFAULT,							// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,							// ウィンドウの左上Ｙ座標 
		_width,							// ウィンドウの幅
		_height,							// ウィンドウの高さ
		NULL,									// 親ウィンドウのハンドル
		NULL,									// メニューハンドルまたは子ウィンドウID
		m_hInstance,								// インスタンスハンドル
		NULL								// ウィンドウ作成データ
	);

	// After creating the window, modify the system menu to remove the close button
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	if (hMenu != NULL) {
		// Disable the close button by removing the SC_CLOSE item from the system menu
		RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
	}

	//ウィンドウのサイズを修正
	RECT rc1, rc2;
	GetWindowRect(hWnd, &rc1);
	GetClientRect(hWnd, &rc2);
	int sx = _width;
	int sy = _height;
	sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
	sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, sx, sy, (SWP_NOZORDER |
		SWP_NOOWNERZORDER | SWP_NOMOVE));

	ShowWindow(hWnd, m_nCmdShow);
	// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
	UpdateWindow(hWnd);

	DirectX11::CreateWindowSwapchain(hWnd);

	m_hwndObjNames.insert(std::make_pair(hWnd, _objName));

	return hWnd;
}

//まだ表示されていないウィンドウハンドル
std::vector<HWND> unShowHwnds;

HWND Window::WindowSubCreateAsync(std::string _objName, std::string _windowName, int _width, int _height)
{
	// Create a promise and future for window handle
	std::promise<HWND> handlePromise;
	std::future<HWND> handleFuture = handlePromise.get_future();

	objectName = _objName;
	windowName = _windowName;
	windowWidth = _width;
	windowHeight = _height;

	// Send a message to the main thread to create the window
	PostMessage(mainHwnd, WM_CREATE_NEW_WINDOW, reinterpret_cast<WPARAM>(&handlePromise), 0);

	// Wait for the window handle to be set in the future
	HWND hNewWindow = handleFuture.get();
	unShowHwnds.push_back(hNewWindow);
	return hNewWindow;
}

void Window::WindowSubRelease(HWND hWnd)
{
	PostMessage(mainHwnd, WM_DELETE_WINDOW, (WPARAM)hWnd, 0);
}

BOOL StackShowWindow(HWND _hwnd, int cCmdShow)
{ 
	unShowHwnds.push_back(_hwnd);
	return false; 
}

void Window::WindowSubLoadingBegin()
{
	pWindowSubCreate = WindowSubCreateAsync;
}

void Window::WindowSubLoadingEnd()
{
	for (auto hwnd : unShowHwnds)
	{
		ShowWindow(hwnd, SW_SHOW);
	}
	unShowHwnds.clear();
}

void Window::WindowSubHide()
{
	for (auto node : m_hwndObjNames)
	{
		ShowWindow(node.first, SW_HIDE);
	}
}

LRESULT Window::WindowInit(void(*p_mainInitFunc)(void))
{
	//初期化処理
	//=================================================
	SceneManager::m_sceneList.clear();
	//Box2Dワールド作成
	Box2D::WorldManager::CreateWorld();

#ifdef BOX2D_UPDATE_MULTITHREAD
	//ワールドの更新スタート
	Box2D::WorldManager::StartWorldUpdate();
#endif

	RenderManager::Init();
	p_mainInitFunc();

	SceneManager::Init();

#ifdef DEBUG_TRUE
	Box2DBodyManager::Init();
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
	const long long worldFrameCount = frequency / WORLD_FPS;
	const long long updateFrameCount = frequency / UPDATE_FPS;

	// ゲームループ
	while(true)
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
		worldLag += (nowCount - worldOldCount);
		worldOldCount = nowCount;
		//FixedUpdate
		if(worldLag >= worldFrameCount)
		{
			worldLag -= worldFrameCount;
#ifndef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::WorldUpdate();
#endif
			TRY_CATCH_LOG(Box2D::WorldManager::ExcuteSensorEvent());

#ifdef DEBUG_TRUE
			worldFpsounter++;
#endif
		}

		updateLag += (nowCount - updateOldCount);
		updateOldCount = nowCount;
		
		//if (nowCount >= previewCount + updateFrameCount)
		if(updateLag >= updateFrameCount)
		{
			updateLag -= updateFrameCount;
			//Animator用のカウント更新
			AnimatorManager::deltaCount = nowCount - worldOldCount;

			Input::Get().Update();

			TRY_CATCH_LOG(SceneManager::m_currentScene->Update());

			TRY_CATCH_LOG(ObjectManager::UpdateObjectComponent());

			Box2DBodyManager::ExcuteMoveFunction();

			//カメラ関連行列セット
			CameraManager::SetCameraMatrix();

			DirectX11::D3D_StartRender();

			RenderManager::Draw();

#ifdef SFTEXT_TRUE
			SFTextManager::ExcuteDrawString();
#endif

#ifdef DEBUG_TRUE
			ImGuiApp::Draw();
#endif

			DirectX11::D3D_FinishRender();

#ifdef DEBUG_TRUE
			updateFpsCounter++;
			nowTick = GetTickCount64();

			if (nowTick >= oldTick + 1000)
			{
				ImGuiApp::worldFpsCounter = worldFpsounter;
				worldFpsounter = 0;
				ImGuiApp::updateFpsCounter = updateFpsCounter;
				updateFpsCounter = 0;
				oldTick = nowTick;
			}
#endif
		}
	}

	return LRESULT();
}

LRESULT Window::WindowUpdate(std::future<void>& sceneFuture,bool& loading)
{
	const long long worldFrameCount = frequency / WORLD_FPS;
	const long long updateFrameCount = frequency / UPDATE_FPS;
	// Example main loop with a loading screen
	bool loadingComplete = false;

	// ゲームループ
	while (!loadingComplete && !terminateFlag)
	{
		// 新たにメッセージがあれば
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
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
			worldLag += (nowCount - worldOldCount);
			worldOldCount = nowCount;
			//FixedUpdate
			//if (nowCount >= worldOldCount + worldFrameCount)
			if (worldLag >= worldFrameCount/* && worldFpsounter <= FIXED_FPS*/)
			{
				worldLag -= worldFrameCount;
#ifndef BOX2D_UPDATE_MULTITHREAD
				Box2D::WorldManager::WorldUpdate();
#endif
				TRY_CATCH_LOG(Box2D::WorldManager::ExcuteSensorEvent());

#ifdef DEBUG_TRUE
				worldFpsounter++;
#endif
			}

			updateLag += (nowCount - updateOldCount);
			updateOldCount = nowCount;

			//if (nowCount >= previewCount + updateFrameCount)
			if (updateLag >= updateFrameCount)
			{
				updateLag -= updateFrameCount;
				//Animator用のカウント更新
				AnimatorManager::deltaCount = nowCount - worldOldCount;

				Input::Get().Update();

				TRY_CATCH_LOG(SceneManager::m_currentScene->Update());

				TRY_CATCH_LOG(ObjectManager::UpdateObjectComponent());

				Box2DBodyManager::ExcuteMoveFunction();

				//カメラ関連行列セット
				CameraManager::SetCameraMatrix();

				DirectX11::D3D_StartRender();

				RenderManager::Draw();

#ifdef SFTEXT_TRUE
				SFTextManager::ExcuteDrawString();
#endif

#ifdef DEBUG_TRUE
				ImGuiApp::Draw();
#endif

				DirectX11::D3D_FinishRender();

#ifdef DEBUG_TRUE
				updateFpsCounter++;
				nowTick = GetTickCount64();

				if (nowTick >= oldTick + 1000)
				{
					ImGuiApp::worldFpsCounter = worldFpsounter;
					worldFpsounter = 0;
					ImGuiApp::updateFpsCounter = updateFpsCounter;
					updateFpsCounter = 0;
					oldTick = nowTick;
				}
#endif
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

int Window::WindowEnd()
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

#ifdef DEBUG_TRUE
	ImGuiApp::Uninit();
#endif

	//DirectXかたずけ
	DirectX11::D3D_Release();

#ifdef DEBUG_TRUE
	//コンソール画面閉じる
	CloseConsoleWindow();
#endif

	UnregisterClass("MAIN_WINDOW", m_hInstance);

	UnregisterClass("SUB_WINDOW", m_hInstance);

	//メモリリーク詳細検知
	//(unique_ptr系はこのあと解放されるので検知されてしまう)
	//_CrtDumpMemoryLeaks();

	return (int)msg.wParam;
}
// Declare isDragging as a static or global variable
bool isDragging = false;

bool pauseGame = false;

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static float screenWindowAspectWidth = 1.0f;
	static float screenWindowAspectHeight = 1.0f;

	switch (uMsg)
	{
	case WM_DESTROY:// ウィンドウ破棄のメッセージ
		PostQuitMessage(0);// 「WM_QUIT」メッセージを送る　→　アプリ終了
		break;

	case WM_CLOSE:  // 「x」ボタンが押されたら
	{
		int res = MessageBoxA(NULL, "終了しますか？", "確認", MB_OKCANCEL);
		if (res == IDOK) 
		{
			terminateFlag = true;
			DestroyWindow(hWnd);  // 「WM_DESTROY」メッセージを送る
		}

		//現在時間を取得
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		worldOldCount = nowCount;
		updateOldCount = nowCount;
	}
	break;

	case WM_PAUSE_GAME:
	{
		QueryPerformanceCounter(&liWork);
		long long pauseNowCount = liWork.QuadPart;
		long long pauseOldCount = pauseNowCount;
		long long pauseFrameCount = frequency / 30;

		pauseGame = true;
		while (pauseGame)
		{
			// 新たにメッセージがあれば
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				// ウィンドウプロシージャにメッセージを送る
				DispatchMessage(&msg);

				// 「WM_QUIT」メッセージを受け取ったらループを抜ける
				if (msg.message == WM_QUIT) {
					break;
				}
			}
			else
			{
				QueryPerformanceCounter(&liWork);
				pauseNowCount = liWork.QuadPart;

				if (pauseNowCount > pauseOldCount + pauseFrameCount)
				{
					pauseOldCount = pauseNowCount;

					//カメラ関連行列セット
					CameraManager::SetCameraMatrix();

					DirectX11::D3D_StartRender();

					RenderManager::Draw();

#ifdef SFTEXT_TRUE
					SFTextManager::KeepExcuteDrawString();
#endif

#ifdef DEBUG_TRUE
					ImGuiApp::Draw();
#endif

					DirectX11::D3D_FinishRender();
				}
			}
		}
	}
	break;

	case WM_RESUME_GAME:
		pauseGame = false;
		//現在時間を取得
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		worldOldCount = nowCount;
		updateOldCount = nowCount;
		break;

#ifdef SUBWINDOW_IS_TOP

	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) != WA_INACTIVE)
		{
			// Post a custom message to adjust the z-order after activation
			PostMessage(hWnd, WM_ADJUST_Z_ORDER, 0, 0);
		}
	}
	break;
	case WM_ADJUST_Z_ORDER:
	{
		RECT rect;
		for (auto hwnd : m_hwndObjNames)
		{
			GetWindowRect(hwnd.first, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			SetWindowPos(hwnd.first, HWND_TOP, rect.left, rect.top, width, height, SWP_NOSIZE);
		}
	}
	break;
#endif

	case WM_CREATE_NEW_WINDOW:
	{
		HWND hWnd = CreateWindowEx(
			0,										// 拡張ウィンドウスタイル
			"SUB_WINDOW",								// ウィンドウクラスの名前
			windowName.c_str(),							// ウィンドウの名前
			WS_OVERLAPPED | WS_CAPTION,// ウィンドウスタイル
			CW_USEDEFAULT,							// ウィンドウの左上Ｘ座標
			CW_USEDEFAULT,							// ウィンドウの左上Ｙ座標 
			windowWidth,							// ウィンドウの幅
			windowHeight,							// ウィンドウの高さ
			NULL,									// 親ウィンドウのハンドル
			NULL,									// メニューハンドルまたは子ウィンドウID
			m_hInstance,								// インスタンスハンドル
			NULL								// ウィンドウ作成データ
		);

		//ウィンドウのサイズを修正
		RECT rc1, rc2;
		GetWindowRect(hWnd, &rc1);
		GetClientRect(hWnd, &rc2);
		int sx = windowWidth;
		int sy = windowHeight;
		sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
		sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, sx, sy, (SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_NOMOVE));

		// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
		UpdateWindow(hWnd);

		DirectX11::CreateWindowSwapchain(hWnd);

		m_hwndObjNames.insert(std::make_pair(hWnd, objectName));

		// Fulfill the promise to return the window handle
		std::promise<HWND>* pPromise = reinterpret_cast<std::promise<HWND>*>(wParam);
		pPromise->set_value(hWnd);
	}
	break;

	case WM_DELETE_WINDOW:
	{
		HWND hTargetWnd = (HWND)wParam;
		auto& swaplist = DirectX11::m_pSwapChainList;
		auto swapiter = swaplist.find(hTargetWnd);
		if (swapiter != swaplist.end())
		{
			swapiter->second.Get()->Release();
			swaplist.erase(swapiter);
		}

		auto& viewlist = DirectX11::m_pRenderTargetViewList;
		auto viewiter = viewlist.find(hTargetWnd);
		if (viewiter != viewlist.end())
		{
			viewiter->second.Get()->Release();
			viewlist.erase(viewiter);
		}

		auto objIter = m_hwndObjNames.find(hTargetWnd);
		if (objIter != m_hwndObjNames.end())
		{
			m_hwndObjNames.erase(objIter);
		}

		DestroyWindow(hTargetWnd);
	}
	break;

	case WM_NCLBUTTONDOWN:

#ifdef MAINWINDOW_LOCK
		if (wParam == HTCLOSE) {
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			//return DefWindowProc(hWnd, uMsg, wParam, lParam);
			return 0;
		}
		else{
			return 0;
		}
#endif

#ifdef BOX2D_UPDATE_MULTITHREAD
		Box2D::WorldManager::pPauseWorldUpdate();
#endif
		isDragging = true;
		SetTimer(hWnd, 1, 50, NULL);
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	case WM_TIMER:
		if (isDragging && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
			// If the left mouse button is no longer down
			isDragging = false;
			KillTimer(hWnd, 1);          // Stop the timer
#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			//現在時間を取得
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart;
			worldOldCount = nowCount;
			updateOldCount = nowCount;
		}
		return 0;

	case WM_EXITSIZEMOVE:
		if (isDragging) {
			isDragging = false;  // Reset the flag
			KillTimer(hWnd, 1);          // Stop the timer
#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			//現在時間を取得
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart;
			worldOldCount = nowCount;
			updateOldCount = nowCount;
		}
		return 0;

	case WM_MOVING:
	{
		//カメラ関連行列セット
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::KeepExcuteDrawString();
#endif

#ifdef DEBUG_TRUE
		ImGuiApp::Draw();
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
		static const LONG HALF_SCREEN_WIDTH = PROJECTION_WIDTH / 2;
		static const LONG HALF_SCREEN_HEIGHT = PROJECTION_HEIGHT / 2;
		Input::mousePoint.x = static_cast<LONG>(Input::mousePoint.x * screenWindowAspectWidth) - HALF_SCREEN_WIDTH;
		Input::mousePoint.y = static_cast<LONG>(Input::mousePoint.y * screenWindowAspectHeight) - HALF_SCREEN_HEIGHT;
		Input::mousePoint.y *= -1;
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
		screenWindowAspectWidth = PROJECTION_WIDTH / static_cast<float>(windowSize.right);
		screenWindowAspectHeight = PROJECTION_HEIGHT / static_cast<float>(windowSize.bottom);
		break;

	case WM_SIZING:
	{
		//カメラ関連行列セット
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::KeepExcuteDrawString();
#endif

#ifdef DEBUG_TRUE
		ImGuiApp::Draw();
#endif

		DirectX11::D3D_FinishRender();
	}
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
			swapiter->second.Get()->Release();
			swaplist.erase(swapiter);
		}

		auto& viewlist = DirectX11::m_pRenderTargetViewList;
		auto viewiter = viewlist.find(hWnd);
		if (viewiter != viewlist.end())
		{
			viewiter->second.Get()->Release();
			viewlist.erase(viewiter);
		}

		auto objIter = m_hwndObjNames.find(hWnd);
		if (objIter != m_hwndObjNames.end())
		{
			m_hwndObjNames.erase(objIter);
		}

		DestroyWindow(hWnd);
	}
	break;

	case WM_NCLBUTTONDOWN:
	{
#ifdef BOX2D_UPDATE_MULTITHREAD
		Box2D::WorldManager::pPauseWorldUpdate();
#endif
		isDragging = true;
		SetTimer(hWnd, 1, 50, NULL);

		auto iter = m_hwndObjNames.find(hWnd);
		if (iter != m_hwndObjNames.end())
		{
			auto gameObject = ObjectManager::Find(iter->second);
			if (gameObject != nullptr)
			{
				const auto& list = gameObject->m_componentList;
				for (auto& component : list)
				{
					component.second->OnWindowEnter(hWnd);
				}
			}
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	case WM_TIMER:
		if (isDragging && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) {
			// If the left mouse button is no longer down
			isDragging = false;
			KillTimer(hWnd, 1);          // Stop the timer
#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			//現在時間を取得
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart;
			worldOldCount = nowCount;
			updateOldCount = nowCount;

			auto iter = m_hwndObjNames.find(hWnd);
			if (iter != m_hwndObjNames.end())
			{
				auto gameObject = ObjectManager::Find(iter->second);
				if (gameObject != nullptr)
				{
					const auto& list = gameObject->m_componentList;
					for (auto& component : list)
					{
						component.second->OnWindowExit(hWnd);
					}
				}
			}
		}
		return 0;

	case WM_EXITSIZEMOVE:
		if (isDragging) {
			isDragging = false;  // Reset the flag
			KillTimer(hWnd, 1);          // Stop the timer
#ifdef BOX2D_UPDATE_MULTITHREAD
			Box2D::WorldManager::pResumeWorldUpdate();
#endif
			//現在時間を取得
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart;
			worldOldCount = nowCount;
			updateOldCount = nowCount;

			auto iter = m_hwndObjNames.find(hWnd);
			if (iter != m_hwndObjNames.end())
			{
				auto gameObject = ObjectManager::Find(iter->second);
				if (gameObject != nullptr)
				{
					const auto& list = gameObject->m_componentList;
					for (auto& component : list)
					{
						component.second->OnWindowExit(hWnd);
					}
				}
			}
		}
		return 0;

	case WM_MOVING:
	{
		RECT* rect = reinterpret_cast<RECT*>(lParam);

		auto iter = m_hwndObjNames.find(hWnd);
		if (iter != m_hwndObjNames.end())
		{
			auto gameObject = ObjectManager::Find(iter->second);
			if (gameObject != nullptr)
			{
				const auto& list = gameObject->m_componentList;
				for (auto& component : list)
				{
					component.second->OnWindowMove(hWnd, rect);
				}
			}
		}

		//カメラ関連行列セット
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::KeepExcuteDrawString();
#endif

#ifdef DEBUG_TRUE
		ImGuiApp::Draw();
#endif

		DirectX11::D3D_FinishRender();

		//RECT* rect = reinterpret_cast<RECT*>(lParam);

		//// Calculate the window width and height
		//int width = rect->right - rect->left;
		//int height = rect->bottom - rect->top;

		//// Adjust position to keep within the left boundary
		//if (rect->left < leftBoundary)
		//{
		//	rect->left = leftBoundary;
		//	rect->right = leftBoundary + width;
		//}

		//// Adjust position to keep within the right boundary
		//if (rect->right > rightBoundary)
		//{
		//	rect->right = rightBoundary;
		//	rect->left = rightBoundary - width;
		//}

		//// Adjust position to keep within the top boundary
		//if (rect->top < topBoundary)
		//{
		//	rect->top = topBoundary;
		//	rect->bottom = topBoundary + height;
		//}

		//// Adjust position to keep within the bottom boundary
		//if (rect->bottom > bottomBoundary)
		//{
		//	rect->bottom = bottomBoundary;
		//	rect->top = bottomBoundary - height;
		//}

		//return TRUE;  // Indicate that we've modified the window position
	}
	break;

	default:
		// 受け取ったメッセージに対してデフォルトの処理を実行
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return LRESULT();
}

Vector2 GetWindowPosition(HWND _hWnd)
{
	RECT rect;
	GetWindowRect(_hWnd, &rect);
	Vector2 pos =
	{
		static_cast<float>(rect.left + rect.right) / 2 - Window::MONITER_HALF_WIDTH,
		static_cast<float>(rect.top + rect.bottom) / -2 + Window::MONITER_HALF_HEIGHT
	};

	return pos;
}

void SetWindowPosition(HWND _hWnd, Vector2 pos)
{
	RECT rect;
	GetWindowRect(_hWnd, &rect);
	int x = static_cast<int>(pos.x) + Window::MONITER_HALF_WIDTH - (rect.right - rect.left) / 2;
	int y = -static_cast<int>(pos.y) + Window::MONITER_HALF_HEIGHT - (rect.bottom - rect.top) / 2;
	SetWindowPos(_hWnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
