
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

void SetConsoleWindowPositionAndSize(int x, int y, int width, int height) {
	// コンソールウィンドウのハンドルを取得
	HWND consoleWindow = GetConsoleWindow();
	if (!consoleWindow) {
		std::cerr << "コンソールウィンドウの取得に失敗しました。" << std::endl;
		return;
	}

	// バッファサイズの設定
	HANDLE consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD bufferSize = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
	SetConsoleScreenBufferSize(consoleOutput, bufferSize);

	// ウィンドウの位置とサイズを設定 (SetWindowPosでも可)
	MoveWindow(consoleWindow, x, y, width * 10, height * 20, TRUE);
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


HINSTANCE m_hInstance;
int m_nCmdShow;
HWND  mainHwnd;
MSG msg;
RECT windowSize;
LARGE_INTEGER liWork;
long long frequency;
long long worldOldCount;
long long updateOldCount;
long long worldLag = 0;
long long updateLag = 0;
int worldFpsounter = 0;		//FPS計測変数
int updateFpsCounter = 0;						//FPS計測変数
long long oldTick = GetTickCount64();	//前回計測時
long long nowTick = oldTick;				//今回計測時
long long nowCount = worldOldCount;

std::atomic<bool> Window::terminateFlag(false);

thread_local HWND(*Window::pWindowSubCreate)(const std::string&, std::string, int, int, Vector2) = WindowSubCreate;

//ウィンドウのハンドルに対応したオブジェクトの名前
std::unordered_map<HWND, const std::string&> Window::m_hwndObjNames;

ImGuiContext* mainContext;

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

	SetConsoleWindowPositionAndSize(0, 860, 192, 8);

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
	//wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MAIN_WINDOW";
	//wc.hIconSm = NULL;
	HICON loadedIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hIcon = loadedIcon;
	wc.hIconSm = loadedIcon;

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

	mainContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(mainContext);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui_ImplWin32_Init(mainHwnd); // hWnd is your main window handle
	ImGui_ImplDX11_Init(DirectX11::m_pDevice.Get(), DirectX11::m_pDeviceContext.Get());

	ImGuiApp::ImGuiSetKeyMap(mainContext);
#endif

	m_hInstance = hInstance;
	m_nCmdShow = nCmdShow;

	return LRESULT();
}

std::string windowName;
std::string& objectName = windowName;
int windowWidth;
int windowHeight;

bool pauseGame = false;
bool pauseDebug = false;

HWND Window::WindowSubCreate(const std::string& _objName, std::string _windowName, int _width, int _height, Vector2 _pos)
{
	HWND hWnd = CreateWindowEx(
		WS_EX_NOACTIVATE,							// 拡張ウィンドウスタイル
		"SUB_WINDOW",								// ウィンドウクラスの名前
		_windowName.c_str(),							// ウィンドウの名前
		WS_OVERLAPPED | WS_CAPTION /*| WS_SYSMENU*/,// ウィンドウスタイル
		CW_USEDEFAULT,							// ウィンドウの左上Ｘ座標
		CW_USEDEFAULT,							// ウィンドウの左上Ｙ座標 
		_width,							// ウィンドウの幅
		_height,							// ウィンドウの高さ
#ifdef DEBUG_TRUE
		NULL,
#else
		mainHwnd,									// 親ウィンドウのハンドル
#endif
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

	SetWindowPosition(hWnd, _pos);

	ShowWindow(hWnd, m_nCmdShow);
	// ウィンドウの状態を直ちに反映(ウィンドウのクライアント領域を更新)
	UpdateWindow(hWnd);

	DirectX11::CreateWindowSwapChain(hWnd);

	m_hwndObjNames.emplace(hWnd, _objName);

#ifdef DEBUG_TRUE
	if (pauseDebug)
	{
		SetWindowMovable(hWnd, false);
		SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
#endif

	return hWnd;
}

//まだ表示されていないウィンドウハンドル
std::vector<HWND> unShowHwnds;

HWND Window::WindowSubCreateAsync(const std::string& _objName, std::string _windowName, int _width, int _height, Vector2 _pos)
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

	SetWindowPosition(hNewWindow, _pos);

#ifdef DEBUG_TRUE
	if (pauseDebug) SetWindowMovable(hNewWindow, false);
#endif

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

void Window::WindowSubHide(HWND _hWnd)
{
	ShowWindow(_hWnd, SW_HIDE);
#ifdef DEBUG_TRUE
	if (pauseDebug)
		SetWindowPos(_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
#endif
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

#ifdef DEBUG_TRUE
	PostMessage(mainHwnd, WM_PAUSE_DEBUG, 0, 0);
#endif
	//=================================================

	return LRESULT();
}

LRESULT Window::WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/)
{
	const long long worldFrameCount = frequency / WORLD_FPS;
	const long long updateFrameCount = frequency / UPDATE_FPS;

	//Animator用のカウント更新
	AnimatorManager::deltaCount = updateFrameCount;

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
			TRY_CATCH_LOG(Box2D::WorldManager::ExecuteBodyEvent());

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
			AnimatorManager::deltaCount = updateFrameCount;

			Input::Get().Update();

			//TRY_CATCH_LOG(SceneManager::m_currentScene->Update());

			TRY_CATCH_LOG(ObjectManager::UpdateObjectComponent());

			//カメラ関連行列セット
			CameraManager::SetCameraMatrix();

			DirectX11::D3D_StartRender();

			RenderManager::Draw();

#ifdef SFTEXT_TRUE
			SFTextManager::Draw();
#endif
#ifdef DEBUG_TRUE
			ImGuiApp::DrawMainGui(mainContext);

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

	//Animator用のカウント更新
	AnimatorManager::deltaCount = updateFrameCount;

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
				TRY_CATCH_LOG(Box2D::WorldManager::ExecuteBodyEvent());

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
				AnimatorManager::deltaCount = updateFrameCount;

				Input::Get().Update();

				//TRY_CATCH_LOG(SceneManager::m_currentScene->Update());

				TRY_CATCH_LOG(ObjectManager::UpdateObjectComponent());

				//カメラ関連行列セット
				CameraManager::SetCameraMatrix();

				DirectX11::D3D_StartRender();

				RenderManager::Draw();

#ifdef SFTEXT_TRUE
				SFTextManager::Draw();
#endif
#ifdef DEBUG_TRUE
				ImGuiApp::DrawMainGui(mainContext);

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
	ObjectManager::UnInit();

	//オブジェクトを削除する(念入りに)
	ObjectManager::CleanAllObjectList();
	
#ifdef DEBUG_TRUE
	ImGuiApp::UnInit();

	ImGui::SetCurrentContext(mainContext);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	//すべてのワールドを削除する
	Box2D::WorldManager::DeleteAllWorld();

	//シーンの破棄
	SceneManager::UnInit();

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

const HWND& Window::GetMainHWnd()
{
	return mainHwnd;
}

void Window::PauseGame()
{
	if (!pauseGame)
	{
		LOG("pause");
		PostMessage(mainHwnd, WM_PAUSE_GAME, 0, 0);
	}
}

void Window::ResumeGame()
{
	if (pauseGame)
	{
		LOG("resume");
		PostMessage(mainHwnd, WM_RESUME_GAME, 0, 0);
	}
}

const bool Window::IsPause()
{
	return pauseGame;
}

#ifdef DEBUG_TRUE
const bool Window::IsPauseDebug()
{
	return pauseDebug;
}
#endif

// Declare isDragging as a static or global variable
bool isDragging = false;

// ウィンドウのZオーダーを取得するためのコールバック
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	std::vector<HWND>* windowList = reinterpret_cast<std::vector<HWND>*>(lParam);
	if (IsWindowVisible(hwnd)) {
		windowList->push_back(hwnd);
	}
	return TRUE;
}

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG_TRUE
	if (mainContext != nullptr)
	{
		ImGui::SetCurrentContext(mainContext);
		ImGuiIO& io = ImGui::GetIO();

		switch (uMsg) {
			// Mouse button down
		case WM_LBUTTONDOWN:
			io.MouseDown[0] = true;  // Left mouse button
			break;
		case WM_RBUTTONDOWN:
			io.MouseDown[1] = true;  // Right mouse button
			break;
		case WM_MBUTTONDOWN:
			io.MouseDown[2] = true;  // Middle mouse button
			break;

			// Mouse button up
		case WM_LBUTTONUP:
			io.MouseDown[0] = false;
			break;
		case WM_RBUTTONUP:
			io.MouseDown[1] = false;
			break;
		case WM_MBUTTONUP:
			io.MouseDown[2] = false;
			break;

			// Mouse movement
		case WM_MOUSEMOVE:
			io.MousePos.x = (float)GET_X_LPARAM(lParam);
			io.MousePos.y = (float)GET_Y_LPARAM(lParam);
			break;

			// Mouse wheel
		case WM_MOUSEWHEEL:
			io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
			break;

#ifndef IMGUI_DISABLE_OBSOLETE_KEYIO
			// Keyboard events
		case WM_KEYDOWN:
			if (wParam < 256)

			break;
		case WM_KEYUP:
			if (wParam < 256)
				io.KeysDown[wParam] = 0;
			break;
#endif
		case WM_CHAR:
			if (wParam > 0 && wParam < 0x10000)
				io.AddInputCharacter((unsigned short)wParam);
			break;
		}

		if (ImGui::GetIO().WantCaptureMouse) {
			// ImGuiがマウス入力を処理する場合
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
	}
#endif

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
		pauseGame = true;

#ifdef DEBUG_TRUE
		if (pauseDebug) break;
#endif

		QueryPerformanceCounter(&liWork);
		long long pauseNowCount = liWork.QuadPart;
		long long pauseOldCount = pauseNowCount;
		long long pauseFrameCount = frequency / 60;

		//Animator用のカウント更新
		AnimatorManager::deltaCount = pauseFrameCount;

		Vector2 oldMousePos;

		
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

					Input::Get().Update();

					TRY_CATCH_LOG(ObjectManager::PauseUpdateObjectComponent());

					//カメラ関連行列セット
					CameraManager::SetCameraMatrix();

					DirectX11::D3D_StartRender();

					RenderManager::Draw();
#ifdef SFTEXT_TRUE
					SFTextManager::Draw();
#endif
#ifdef DEBUG_TRUE
					ImGuiApp::DrawMainGui(mainContext);

					ImGuiApp::Draw();
#endif
					DirectX11::D3D_FinishRender();

#ifdef DEBUG_TRUE
					updateFpsCounter++;
					nowTick = GetTickCount64();

					if (nowTick >= oldTick + 1000)
					{
						ImGuiApp::updateFpsCounter = updateFpsCounter;
						updateFpsCounter = 0;
						oldTick = nowTick;
					}
#endif
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

#ifdef DEBUG_TRUE
	case WM_PAUSE_DEBUG:
	{
		QueryPerformanceCounter(&liWork);
		long long pauseNowCount = liWork.QuadPart;
		long long pauseOldCount = pauseNowCount;
		long long pauseFrameCount = frequency / 60;

		//Animator用のカウント更新
		AnimatorManager::deltaCount = pauseFrameCount;

		Vector2 oldMousePos;

		for (auto hWnd : m_hwndObjNames)
		{
			SetWindowMovable(hWnd.first, false);
		}

		SetWindowPos(mainHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		pauseDebug = true;
		while (pauseDebug)
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

					Input::Get().Update();					

					Vector2 mousePos = Input::Get().MousePoint();

					Vector2 worldPos = mousePos;
					worldPos.x = worldPos.x * DISPALY_ASPECT_WIDTH / RenderManager::renderZoom.x + RenderManager::renderOffset.x;
					worldPos.y = worldPos.y * DISPALY_ASPECT_HEIGHT / RenderManager::renderZoom.y + RenderManager::renderOffset.y;				

					worldPos.x *= PROJECTION_ASPECT_WIDTH;
					worldPos.y *= PROJECTION_ASPECT_HEIGHT;

					bool handleHit = ImGuiApp::UpdateHandleUI(worldPos);
				
					bool inGameScreen = abs(mousePos.x) <= PROJECTION_WIDTH / 2 && abs(mousePos.y) <= PROJECTION_HEIGHT / 2;
					if (!ImGuiApp::handleUi.lock)
					{
						//if(abs(mousePos.x) <= PROJECTION_WIDTH / 2 && abs(mousePos.y) <= PROJECTION_HEIGHT / 2)
						{
							std::list<GameObject*> targetObjects;

							//Box2DのShapeに当たるかどうか
							//========================================================================================
							static float range = 0.8f;
							b2AABB aabb;
							aabb.lowerBound =
							{ worldPos.x / DEFAULT_OBJECT_SIZE - range,worldPos.y / DEFAULT_OBJECT_SIZE - range };
							aabb.upperBound =
							{ worldPos.x / DEFAULT_OBJECT_SIZE + range,worldPos.y / DEFAULT_OBJECT_SIZE + range };
							/*	b2Circle circle;
							circle.center = { 0.0f,0.0f};
							circle.radius = 1.0f;
							b2Transform tf;
							tf.p = { worldPos.x / DEFAULT_OBJECT_SIZE,worldPos.y / DEFAULT_OBJECT_SIZE };
							tf.q = b2Rot_identity;*/
							std::vector<b2ShapeId> outputShape;
							auto filter = b2DefaultQueryFilter();
							filter.categoryBits = ALL_BITS;
							b2World_OverlapAABB(*Box2D::WorldManager::currentWorldId, aabb,
								filter, Box2D::OverlapResultVectorb2ShapeId, &outputShape);
							for (auto& shape : outputShape)
							{
								b2BodyId bodyId = b2Shape_GetBody(shape);
								auto iter = Box2DBodyManager::m_bodyObjectName.find(bodyId.index1);
								if (iter != Box2DBodyManager::m_bodyObjectName.end())
								{
									auto object = ObjectManager::Find(iter->second);
									if (object != nullptr)
									{
										if (!object->selectable) continue;
										auto it = std::find(targetObjects.begin(), targetObjects.end(), object);
										if (it == targetObjects.end())
											targetObjects.push_back(object);
									}
								}
							}
							//========================================================================================

							for (auto& object : ObjectManager::m_objectList->first)
							{
								if (!object->active || !object->selectable) continue;

								if (object->ExistComponent<Box2DBody>()) continue;

								auto it = std::find(targetObjects.begin(), targetObjects.end(), object.get());
								if (it != targetObjects.end()) continue;

								const Vector2& pos = object->transform.position;
								Vector2 scale = object->transform.scale;
								scale *= HALF_OBJECT_SIZE;
								if ((pos.x - scale.x) < worldPos.x &&
									(pos.x + scale.x) > worldPos.x &&
									(pos.y - scale.y) < worldPos.y &&
									(pos.y + scale.y) > worldPos.y)
								{
									targetObjects.push_back(object.get());
								}
							}
							static GameObject* target = nullptr;
							if (target != nullptr)
							{
								if (target->isSelected != GameObject::SELECTED ||
									(handleHit && target->isSelected != GameObject::SELECTED))
								{
									target->isSelected = GameObject::SELECT_NONE;
								}
							}

							if (!handleHit && inGameScreen)
							{

								if (targetObjects.empty())
								{
									if (Input::Get().MouseLeftTrigger())
									{
										ImGuiApp::InvalidSelectedObject();
									}
								}
								else
								{
									targetObjects.sort([&](const GameObject* a, const GameObject* b)
										{return a->selectedNum > b->selectedNum; });

									target = targetObjects.front();
									if (target != nullptr && target->isSelected != GameObject::SELECTED)
										target->isSelected = GameObject::ON_MOUSE;

									if (Input::Get().MouseLeftTrigger())
									{
										ImGuiApp::InvalidSelectedObject();
										if (target != nullptr)target->selectedNum = 0;
										ImGuiApp::SetSelectedObject(target);
										targetObjects.pop_front();
										for (auto& object : targetObjects)
										{
											object->selectedNum++;
										}
									}
								}
							}
						}

						if (Input::Get().KeyTrigger(VK_DELETE))
						{
							ImGuiApp::DeleteSelectedObject();
						}

						if (Input::Get().KeyTrigger(VK_W))
						{
							ImGuiApp::handleUi.handleMode = ImGuiApp::HandleUI::POSITION;
							ImGuiApp::handleUi.moveMode = ImGuiApp::HandleUI::NONE;
						}
						else if (Input::Get().KeyTrigger(VK_E))
						{
							ImGuiApp::handleUi.handleMode = ImGuiApp::HandleUI::ROTATION;
							ImGuiApp::handleUi.moveMode = ImGuiApp::HandleUI::NONE;
						}
						else if (Input::Get().KeyTrigger(VK_R))
						{
							ImGuiApp::handleUi.handleMode = ImGuiApp::HandleUI::SCALE;
							ImGuiApp::handleUi.moveMode = ImGuiApp::HandleUI::NONE;
						}

						if (Input::Get().KeyPress(VK_CONTROL))
						{
							if (Input::Get().KeyTrigger(VK_C))
							{
								ImGuiApp::CopySelectedObject();
							}
							if (Input::Get().KeyTrigger(VK_V))
							{
								ImGuiApp::PastSelectedObject();
							}
							if (Input::Get().KeyTrigger(VK_X))
							{
								ImGuiApp::CopySelectedObject();
								ImGuiApp::DeleteSelectedObject();
							}
							if (Input::Get().KeyTrigger(VK_D))
							{
								ImGuiApp::CloneSelectedObject();
							}
							if (Input::Get().KeyTrigger(VK_Z))
							{
								ImGuiApp::RewindChange();
							}
						}
					}


					if (inGameScreen)
					{
						if (Input::Get().MouseRightTrigger())
						{
							oldMousePos = mousePos;
						}

						if (Input::Get().MouseRightPress())
						{
							Vector2 dis = mousePos - oldMousePos;
							oldMousePos = mousePos;
							dis *= -1.0f;
							float rad = -Math::DegToRad(CameraManager::cameraRotation);
							Vector2 offset;
							offset.x = dis.x * cos(rad) - dis.y * sin(rad);
							offset.y = dis.x * sin(rad) + dis.y * cos(rad);
							RenderManager::renderOffset += offset / RenderManager::renderZoom;
						}

						if (Input::Get().MouseWheelDelta() > 0)
						{
							RenderManager::renderZoom += RenderManager::renderZoom / 100;
						}
						else if (Input::Get().MouseWheelDelta() < 0)
						{
							RenderManager::renderZoom -= RenderManager::renderZoom / 100;
						}
					}

					//カメラ関連行列セット
					CameraManager::SetCameraMatrix();

					DirectX11::D3D_StartRender();

					RenderManager::Draw();

					ImGuiApp::DrawHandleUI(worldPos);
#ifdef SFTEXT_TRUE
					SFTextManager::Draw();
#endif
					ImGuiApp::DrawMainGui(mainContext);

					ImGuiApp::Draw();
					DirectX11::D3D_FinishRender();
				}
			}
		}

		for (auto hwnd : m_hwndObjNames)
		{
			SetWindowMovable(hwnd.first, true);

#ifdef SUBWINDOW_IS_TOP
			SetWindowPos(hwnd.first, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif
		}

		if (pauseGame)
		{
			PostMessage(mainHwnd, WM_PAUSE_GAME, 0, 0);
		}
	}
	break;

	case WM_RESUME_DEBUG:
		pauseDebug = false;
		//現在時間を取得
		QueryPerformanceCounter(&liWork);
		nowCount = liWork.QuadPart;
		worldOldCount = nowCount;
		updateOldCount = nowCount;

		RenderManager::renderOffset = { 0,0 };
		RenderManager::renderZoom = { 1.0f,1.0f };
		break;
#endif

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
#ifdef DEBUG_TRUE
		/*HWND consoleWindow = GetConsoleWindow();
		SetWindowPos(consoleWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);*/

		HWND hNextWnd = GetNextWindow(mainHwnd, GW_HWNDNEXT);
		for (auto hwnd : ImGuiApp::m_hWnd)
		{
			if (hNextWnd != hwnd) {
				SetWindowPos(hwnd, mainHwnd, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		}

		if (pauseDebug) break;

		for (auto hwnd : m_hwndObjNames)
		{
			SetWindowPos(hwnd.first, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
#endif
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
#ifdef DEBUG_TRUE
			NULL,
#else
			mainHwnd,									// 親ウィンドウのハンドル
#endif
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

		DirectX11::CreateWindowSwapChain(hWnd);

		m_hwndObjNames.emplace(hWnd, objectName);

		// Fulfill the promise to return the window handle
		std::promise<HWND>* pPromise = reinterpret_cast<std::promise<HWND>*>(wParam);
		pPromise->set_value(hWnd);
	}
	break;

	case WM_DELETE_WINDOW:
	{
		HWND hTargetWnd = (HWND)wParam;
		auto& swapList = DirectX11::m_pSwapChainList;
		auto swapIter = swapList.find(hTargetWnd);
		if (swapIter != swapList.end())
		{
			swapIter->second.Get()->Release();
			swapList.erase(swapIter);
		}

		auto& viewList = DirectX11::m_pRenderTargetViewList;
		auto viewIter = viewList.find(hTargetWnd);
		if (viewIter != viewList.end())
		{
			viewIter->second.first.Get()->Release();
			viewList.erase(viewIter);
		}

		auto& waveList = DirectX11::m_waveHandleList;
		auto waveIter = waveList.find(hTargetWnd);
		if (waveIter != waveList.end())
		{
			waveList.erase(waveIter);
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
		SFTextManager::Draw();
#endif

#ifdef DEBUG_TRUE
		ImGuiApp::DrawMainGui(mainContext);

		ImGuiApp::Draw();
#endif

		DirectX11::D3D_FinishRender();
	}
	break;

	//case WM_KEYDOWN: //キー入力があったメッセージ
	//	if (LOWORD(wParam) == VK_ESCAPE) { //入力されたキーがESCAPEなら
	//		PostMessage(hWnd, WM_CLOSE, wParam, lParam);//「WM_CLOSE」を送る
	//	}
	//	break;

	case WM_MOUSEMOVE:
	{
		GetCursorPos(&Input::mousePoint);
		ScreenToClient(hWnd, &Input::mousePoint);
		static const LONG HALF_SCREEN_WIDTH = PROJECTION_WIDTH / 2;
		static const LONG HALF_SCREEN_HEIGHT = PROJECTION_HEIGHT / 2;
#ifdef DEBUG_TRUE
		if (ImGuiApp::showMainEditor)
		{
			Input::mousePoint.x = static_cast<LONG>(Input::mousePoint.x * screenWindowAspectWidth * 2) - HALF_SCREEN_WIDTH * 2;
			Input::mousePoint.y = static_cast<LONG>(Input::mousePoint.y * screenWindowAspectHeight * 2) - HALF_SCREEN_HEIGHT;
		}
		else
		{
			Input::mousePoint.x = static_cast<LONG>(Input::mousePoint.x * screenWindowAspectWidth) - HALF_SCREEN_WIDTH;
			Input::mousePoint.y = static_cast<LONG>(Input::mousePoint.y * screenWindowAspectHeight) - HALF_SCREEN_HEIGHT;
		}
#else
		Input::mousePoint.x = static_cast<LONG>(Input::mousePoint.x * screenWindowAspectWidth) - HALF_SCREEN_WIDTH;
		Input::mousePoint.y = static_cast<LONG>(Input::mousePoint.y * screenWindowAspectHeight) - HALF_SCREEN_HEIGHT;
#endif
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

	case WM_MBUTTONDOWN:
		Input::isMouseMiddle = true;
		Input::isMouseDownMiddle = true;
		break;
	case WM_MBUTTONUP:
		Input::isMouseMiddle = false;
		Input::isMouseUpMiddle = true;
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
		SFTextManager::Draw();
#endif

#ifdef DEBUG_TRUE
		ImGuiApp::DrawMainGui(mainContext);

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
#ifdef SUBWINDOW_MOVELOCK
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MOVE) {
			// 移動を無効化
			return 0;
		}
		break;
#endif
	case WM_CLOSE:  // 「x」ボタンが押されたら
	{
		auto& swapList = DirectX11::m_pSwapChainList;
		auto swapIter = swapList.find(hWnd);
		if (swapIter != swapList.end())
		{
			swapIter->second.Get()->Release();
			swapList.erase(swapIter);
		}

		auto& viewList = DirectX11::m_pRenderTargetViewList;
		auto viewIter = viewList.find(hWnd);
		if (viewIter != viewList.end())
		{
			viewIter->second.first.Get()->Release();
			viewList.erase(viewIter);
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
//#ifdef MAINWINDOW_LOCK
//		if (wParam == HTCLOSE) {
//			PostMessage(hWnd, WM_CLOSE, 0, 0);
//			//return DefWindowProc(hWnd, uMsg, wParam, lParam);
//			return 0;
//		}
//		else {
//			return 0;
//		}
//#endif

#ifdef BOX2D_UPDATE_MULTITHREAD
		Box2D::WorldManager::pPauseWorldUpdate();
#endif
		isDragging = true;
		SetTimer(hWnd, 1, 10, NULL);

		auto iter = m_hwndObjNames.find(hWnd);
		if (iter != m_hwndObjNames.end())
		{
			auto gameObject = ObjectManager::Find(iter->second);
			if (gameObject != nullptr)
			{
				const auto& list = gameObject->m_componentList;
				for (auto& component : list.first)
				{
					TRY_CATCH_LOG(component->OnWindowEnter(hWnd));
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
					for (auto& component : list.first)
					{
						TRY_CATCH_LOG(component->OnWindowExit(hWnd));
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
					for (auto& component : list.first)
					{
						TRY_CATCH_LOG(component->OnWindowExit(hWnd));
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
				for (auto& component : list.first)
				{
					TRY_CATCH_LOG(component->OnWindowMove(hWnd, rect));
				}
			}
		}

		//カメラ関連行列セット
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::Draw();
#endif

#ifdef DEBUG_TRUE
		ImGuiApp::DrawMainGui(mainContext);

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

	pos.x *= PROJECTION_ASPECT_WIDTH;
	pos.y *= PROJECTION_ASPECT_HEIGHT;

#ifdef CAMERA_ON_WINDOW
	pos += RenderManager::renderOffset;
#endif

	return pos;
}

void SetWindowPosition(HWND _hWnd, Vector2 pos)
{
	RECT rect;
	GetWindowRect(_hWnd, &rect);

#ifdef CAMERA_ON_WINDOW
	pos -= RenderManager::renderOffset;
#endif

	pos.x /= PROJECTION_ASPECT_WIDTH;
	pos.y /= PROJECTION_ASPECT_HEIGHT;

	int x = static_cast<int>(pos.x) + Window::MONITER_HALF_WIDTH - (rect.right - rect.left) / 2;
	int y = -static_cast<int>(pos.y) + Window::MONITER_HALF_HEIGHT - (rect.bottom - rect.top) / 2;
	
	SetWindowPos(_hWnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void Window::SetWindowMovable(HWND hwnd, bool movable) {
	LONG style = GetWindowLong(hwnd, GWL_STYLE);

	if (movable) {
		// Add the title bar and resize border back
		style |= (WS_CAPTION /*| WS_THICKFRAME*/);
	}
	else {
		// Remove the title bar and resize border
		style &= ~(WS_CAPTION | WS_THICKFRAME);
	}

	SetWindowLong(hwnd, GWL_STYLE, style);

//	auto iter = m_hwndObjNames.find(hwnd);
//	if (iter != m_hwndObjNames.end())
//	{
//		auto object = ObjectManager::Find(iter->second);
//		if (object != nullptr)
//		{
//			RECT rect;
//			GetWindowRect(hwnd, &rect);
//			Vector2 pos = object->transform.position;
//#ifdef CAMERA_ON_WINDOW
//			pos -= RenderManager::renderOffset;
//#endif
//			int x = static_cast<int>(pos.x) + Window::MONITER_HALF_WIDTH - (rect.right - rect.left) / 2;
//			int y = -static_cast<int>(pos.y) + Window::MONITER_HALF_HEIGHT - (rect.bottom - rect.top) / 2;
//
//			Vector2 size = object->transform.scale;
//			int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE * RenderManager::renderZoom.x);
//			int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE * RenderManager::renderZoom.y);
//
//			SetWindowPos(hwnd, nullptr, x, y, width, height, /*SWP_NOMOVE |*/ /*SWP_NOSIZE | */SWP_FRAMECHANGED);
//		}
//	}
}

