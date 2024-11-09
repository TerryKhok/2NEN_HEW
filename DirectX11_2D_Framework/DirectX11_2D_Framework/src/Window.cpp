

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
//���C�������̏I���t���O
std::atomic<bool> Window::mainLoopRun;
#endif

//���j�^�[�̉𑜓x����
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
int Window::worldFpsounter = 0;		//FPS�v���ϐ�
int Window::updateFpsCounter = 0;						//FPS�v���ϐ�
long long Window::oldTick = GetTickCount64();	//�O��v����
long long Window::nowTick = oldTick;				//����v����
long long Window::nowCount = worldOldCount;

std::atomic<bool> Window::terminateFlag(false);

thread_local HWND(*Window::pWindowSubCreate)(std::string, std::string, int, int) = WindowSubCreate;

//�E�B���h�E�̃n���h���ɑΉ������I�u�W�F�N�g�̖��O
std::unordered_map<HWND, std::string> Window::m_hwndObjNames;

LRESULT Window::WindowMainCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Debug��Release��Dpi�𓝈ꂷ�邽��
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	//���j�^�[�̉𑜓x����
	MONITER_HALF_WIDTH = GetSystemMetrics(SM_CXSCREEN) / 2;
	MONITER_HALF_HEIGHT = GetSystemMetrics(SM_CYSCREEN) / 2;

#ifdef DEBUG_TRUE
	//�R���\�[����ʋN��
	CreateConsoleWindow();

	//���������[�N���m
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// �E�B���h�E�N���X�����܂Ƃ߂�
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

	// �E�B���h�E�̏����܂Ƃ߂�
	mainHwnd = CreateWindowEx(
		0, // Extended styles,// �g���E�B���h�E�X�^�C��
		"MAIN_WINDOW",								// �E�B���h�E�N���X�̖��O
		WINDOW_NAME,									// �E�B���h�E�̖��O
#ifdef MAINWINDOW_LOCK
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,	// �E�B���h�E�X�^�C��
#else
		WS_OVERLAPPEDWINDOW,
#endif
		CW_USEDEFAULT,							// �E�B���h�E�̍���w���W
		CW_USEDEFAULT,							// �E�B���h�E�̍���x���W 
		SCREEN_WIDTH,							// �E�B���h�E�̕�
		SCREEN_HEIGHT,							// �E�B���h�E�̍���
		NULL,									// �e�E�B���h�E�̃n���h��
		NULL,									// ���j���[�n���h���܂��͎q�E�B���h�EID
		hInstance,								// �C���X�^���X�n���h��
		NULL);									// �E�B���h�E�쐬�f�[�^

	//�E�B���h�E�̃T�C�Y���C��
	RECT rc1, rc2;
	GetWindowRect(mainHwnd, &rc1);
	GetClientRect(mainHwnd, &rc2);
	int sx = SCREEN_WIDTH;
	int sy = SCREEN_HEIGHT;
	sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
	sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
	SetWindowPos(mainHwnd, NULL, 0, 0, sx, sy, (SWP_NOZORDER |
		SWP_NOOWNERZORDER | SWP_NOMOVE));

	// �w�肳�ꂽ�E�B���h�E�̕\����Ԃ�ݒ�(�E�B���h�E��\��)
	ShowWindow(mainHwnd, nCmdShow);
	// �E�B���h�E�̏�Ԃ𒼂��ɔ��f(�E�B���h�E�̃N���C�A���g�̈���X�V)
	UpdateWindow(mainHwnd);

	SetWindowPosition(mainHwnd, { 0.0f,0.0f });

	//FPS�Œ�p�ϐ�
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	worldOldCount = liWork.QuadPart;
	updateOldCount = liWork.QuadPart;

	//DirectX����
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
		0,										// �g���E�B���h�E�X�^�C��
		"SUB_WINDOW",								// �E�B���h�E�N���X�̖��O
		_windowName.c_str(),							// �E�B���h�E�̖��O
		WS_OVERLAPPED | WS_CAPTION /*| WS_SYSMENU*/,// �E�B���h�E�X�^�C��
		CW_USEDEFAULT,							// �E�B���h�E�̍���w���W
		CW_USEDEFAULT,							// �E�B���h�E�̍���x���W 
		_width,							// �E�B���h�E�̕�
		_height,							// �E�B���h�E�̍���
		NULL,									// �e�E�B���h�E�̃n���h��
		NULL,									// ���j���[�n���h���܂��͎q�E�B���h�EID
		m_hInstance,								// �C���X�^���X�n���h��
		NULL								// �E�B���h�E�쐬�f�[�^
	);

	// After creating the window, modify the system menu to remove the close button
	HMENU hMenu = GetSystemMenu(hWnd, FALSE);
	if (hMenu != NULL) {
		// Disable the close button by removing the SC_CLOSE item from the system menu
		RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
	}

	//�E�B���h�E�̃T�C�Y���C��
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
	// �E�B���h�E�̏�Ԃ𒼂��ɔ��f(�E�B���h�E�̃N���C�A���g�̈���X�V)
	UpdateWindow(hWnd);

	DirectX11::CreateWindowSwapchain(hWnd);

	m_hwndObjNames.insert(std::make_pair(hWnd, _objName));

	return hWnd;
}

//�܂��\������Ă��Ȃ��E�B���h�E�n���h��
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
	//����������
	//=================================================
	SceneManager::m_sceneList.clear();
	//Box2D���[���h�쐬
	Box2D::WorldManager::CreateWorld();

#ifdef BOX2D_UPDATE_MULTITHREAD
	//���[���h�̍X�V�X�^�[�g
	Box2D::WorldManager::StartWorldUpdate();
#endif

	RenderManager::Init();
	p_mainInitFunc();

	SceneManager::Init();

#ifdef DEBUG_TRUE
	Box2DBodyManager::Init();
#endif 

#ifdef SFTEXT_TRUE
	//�e�L�X�g������(Font����)
	SFTextManager::Init();
#endif

	//=================================================

	return LRESULT();
}

LRESULT Window::WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/)
{
	const long long worldFrameCount = frequency / WORLD_FPS;
	const long long updateFrameCount = frequency / UPDATE_FPS;

	// �Q�[�����[�v
	while(true)
	{
		// �V���Ƀ��b�Z�[�W�������
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			// �E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
			DispatchMessage(&msg);

			// �uWM_QUIT�v���b�Z�[�W���󂯎�����烋�[�v�𔲂���
			if (msg.message == WM_QUIT) {
				break;
			}

			continue;
		}
		
		//���ݎ��Ԃ��擾
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
			//Animator�p�̃J�E���g�X�V
			AnimatorManager::deltaCount = nowCount - worldOldCount;

			Input::Get().Update();

			TRY_CATCH_LOG(SceneManager::m_currentScene->Update());

			TRY_CATCH_LOG(ObjectManager::UpdateObjectComponent());

			Box2DBodyManager::ExcuteMoveFunction();

			//�J�����֘A�s��Z�b�g
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

	// �Q�[�����[�v
	while (!loadingComplete && !terminateFlag)
	{
		// �V���Ƀ��b�Z�[�W�������
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			// �E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
			DispatchMessage(&msg);

			// �uWM_QUIT�v���b�Z�[�W���󂯎�����烋�[�v�𔲂���
			if (msg.message == WM_QUIT) {
				break;
			}
		}
		else
		{
			//���ݎ��Ԃ��擾
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
				//Animator�p�̃J�E���g�X�V
				AnimatorManager::deltaCount = nowCount - worldOldCount;

				Input::Get().Update();

				TRY_CATCH_LOG(SceneManager::m_currentScene->Update());

				TRY_CATCH_LOG(ObjectManager::UpdateObjectComponent());

				Box2DBodyManager::ExcuteMoveFunction();

				//�J�����֘A�s��Z�b�g
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
		//�e�N�X�`���ǂݍ��݂Ȃ��ɐݒ肷��
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
	//�I�u�W�F�N�g�̔j��
	ObjectManager::Uninit();

	//�I�u�W�F�N�g���폜����(�O�����)
	ObjectManager::CleanAllObjectList();
	
	//���ׂẴ��[���h���폜����
	Box2D::WorldManager::DeleteAllWorld();

	//�V�[���̔j��
	SceneManager::Uninit();

#ifdef DEBUG_TRUE
	ImGuiApp::Uninit();
#endif

	//DirectX��������
	DirectX11::D3D_Release();

#ifdef DEBUG_TRUE
	//�R���\�[����ʕ���
	CloseConsoleWindow();
#endif

	UnregisterClass("MAIN_WINDOW", m_hInstance);

	UnregisterClass("SUB_WINDOW", m_hInstance);

	//���������[�N�ڍ׌��m
	//(unique_ptr�n�͂��̂��Ɖ�������̂Ō��m����Ă��܂�)
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
	case WM_DESTROY:// �E�B���h�E�j���̃��b�Z�[�W
		PostQuitMessage(0);// �uWM_QUIT�v���b�Z�[�W�𑗂�@���@�A�v���I��
		break;

	case WM_CLOSE:  // �ux�v�{�^���������ꂽ��
	{
		int res = MessageBoxA(NULL, "�I�����܂����H", "�m�F", MB_OKCANCEL);
		if (res == IDOK) 
		{
			terminateFlag = true;
			DestroyWindow(hWnd);  // �uWM_DESTROY�v���b�Z�[�W�𑗂�
		}

		//���ݎ��Ԃ��擾
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
			// �V���Ƀ��b�Z�[�W�������
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				// �E�B���h�E�v���V�[�W���Ƀ��b�Z�[�W�𑗂�
				DispatchMessage(&msg);

				// �uWM_QUIT�v���b�Z�[�W���󂯎�����烋�[�v�𔲂���
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

					//�J�����֘A�s��Z�b�g
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
		//���ݎ��Ԃ��擾
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
			0,										// �g���E�B���h�E�X�^�C��
			"SUB_WINDOW",								// �E�B���h�E�N���X�̖��O
			windowName.c_str(),							// �E�B���h�E�̖��O
			WS_OVERLAPPED | WS_CAPTION,// �E�B���h�E�X�^�C��
			CW_USEDEFAULT,							// �E�B���h�E�̍���w���W
			CW_USEDEFAULT,							// �E�B���h�E�̍���x���W 
			windowWidth,							// �E�B���h�E�̕�
			windowHeight,							// �E�B���h�E�̍���
			NULL,									// �e�E�B���h�E�̃n���h��
			NULL,									// ���j���[�n���h���܂��͎q�E�B���h�EID
			m_hInstance,								// �C���X�^���X�n���h��
			NULL								// �E�B���h�E�쐬�f�[�^
		);

		//�E�B���h�E�̃T�C�Y���C��
		RECT rc1, rc2;
		GetWindowRect(hWnd, &rc1);
		GetClientRect(hWnd, &rc2);
		int sx = windowWidth;
		int sy = windowHeight;
		sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
		sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, sx, sy, (SWP_NOZORDER |
			SWP_NOOWNERZORDER | SWP_NOMOVE));

		// �E�B���h�E�̏�Ԃ𒼂��ɔ��f(�E�B���h�E�̃N���C�A���g�̈���X�V)
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
			//���ݎ��Ԃ��擾
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
			//���ݎ��Ԃ��擾
			QueryPerformanceCounter(&liWork);
			nowCount = liWork.QuadPart;
			worldOldCount = nowCount;
			updateOldCount = nowCount;
		}
		return 0;

	case WM_MOVING:
	{
		//�J�����֘A�s��Z�b�g
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

	case WM_KEYDOWN: //�L�[���͂����������b�Z�[�W
		if (LOWORD(wParam) == VK_ESCAPE) { //���͂��ꂽ�L�[��ESCAPE�Ȃ�
			PostMessage(hWnd, WM_CLOSE, wParam, lParam);//�uWM_CLOSE�v�𑗂�
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

		//��ʂ̑傫���擾
	case WM_SIZE:
		GetClientRect(hWnd, &windowSize);
		screenWindowAspectWidth = PROJECTION_WIDTH / static_cast<float>(windowSize.right);
		screenWindowAspectHeight = PROJECTION_HEIGHT / static_cast<float>(windowSize.bottom);
		break;

	case WM_SIZING:
	{
		//�J�����֘A�s��Z�b�g
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
		// �󂯎�������b�Z�[�W�ɑ΂��ăf�t�H���g�̏��������s
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}

LRESULT Window::WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:  // �ux�v�{�^���������ꂽ��
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
			//���ݎ��Ԃ��擾
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
			//���ݎ��Ԃ��擾
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

		//�J�����֘A�s��Z�b�g
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
		// �󂯎�������b�Z�[�W�ɑ΂��ăf�t�H���g�̏��������s
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
