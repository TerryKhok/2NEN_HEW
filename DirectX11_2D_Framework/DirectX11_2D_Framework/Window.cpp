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
int Window::fpsCounter = 0;						//FPS�v���ϐ�
long long Window::oldTick = GetTickCount64();	//�O��v����
long long Window::nowTick = oldTick;				//����v����
long long Window::nowCount = oldCount;

std::atomic<bool> Window::terminateFlag(false);

//�E�B���h�E�𓮂����Ă��邩�̃t���O
std::atomic<bool> Window::windowSizeMove(false);

LRESULT Window::WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

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
		0,										// �g���E�B���h�E�X�^�C��
		"MAIN_WINDOW",								// �E�B���h�E�N���X�̖��O
		WINDOW_NAME,									// �E�B���h�E�̖��O
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,// �E�B���h�E�X�^�C��
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

	//FPS�Œ�p�ϐ�
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	oldCount = liWork.QuadPart;

	//DirectX����
	DirectX11::D3D_Create(mainHwnd);



	return LRESULT();
}

LRESULT Window::WindowSubCreate(HINSTANCE hInstance, int nCmdShow, const char* _windowName)
{

	HWND hWnd = CreateWindowEx(
		0,										// �g���E�B���h�E�X�^�C��
		"SUB_WINDOW",								// �E�B���h�E�N���X�̖��O
		_windowName,							// �E�B���h�E�̖��O
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,// �E�B���h�E�X�^�C��
		CW_USEDEFAULT,							// �E�B���h�E�̍���w���W
		CW_USEDEFAULT,							// �E�B���h�E�̍���x���W 
		SUB_SCREEN_WIDTH,							// �E�B���h�E�̕�
		SUB_SCREEN_HEIGHT,							// �E�B���h�E�̍���
		NULL,									// �e�E�B���h�E�̃n���h��
		NULL,									// ���j���[�n���h���܂��͎q�E�B���h�EID
		hInstance,								// �C���X�^���X�n���h��
		NULL								// �E�B���h�E�쐬�f�[�^
	);

	//�E�B���h�E�̃T�C�Y���C��
	RECT rc1, rc2;
	GetWindowRect(hWnd, &rc1);
	GetClientRect(hWnd, &rc2);
	int sx = SUB_SCREEN_WIDTH;
	int sy = SUB_SCREEN_HEIGHT;
	sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
	sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
	SetWindowPos(hWnd, NULL, 0, 0, sx, sy, (SWP_NOZORDER |
		SWP_NOOWNERZORDER | SWP_NOMOVE));

	// �w�肳�ꂽ�E�B���h�E�̕\����Ԃ�ݒ�(�E�B���h�E��\��)
	ShowWindow(hWnd, nCmdShow);
	// �E�B���h�E�̏�Ԃ𒼂��ɔ��f(�E�B���h�E�̃N���C�A���g�̈���X�V)
	UpdateWindow(hWnd);

	//���X�g�ɒǉ�
	//hWnds.push_back(hWnd);

	DirectX11::CreateWindowSwapchain(hWnd);

	return LRESULT();
}

LRESULT Window::WindowInit(void(*p_mainInitFunc)(void))
{
	//����������
	//=================================================

	SceneManager::m_sceneList.clear();
	//Box2D���[���h�쐬
	Box2D::WorldManager::CreateWorld();

	RenderManager::Init();
	p_mainInitFunc();

	SceneManager::Init();

#ifdef DEBUG_TRUE
	Box2DBodyManager::Init();
#endif 

#ifdef BOX2D_UPDATE_MULTITHREAD
	//���[���h�̍X�V�X�^�[�g
	Box2D::WorldManager::StartWorldUpdate();
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
	const long long frameCount = frequency / FPS;
	// �Q�[�����[�v
	while (1)
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
		if (nowCount >= oldCount + frameCount)
		{
			//Animator�p�̃J�E���g�X�V
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
			//��O�L���b�`(nullptr�Q�ƂƂ�)
			catch (const std::exception& e) {
				LOG_ERROR(e.what());
			}
#else
			SceneManager::m_currentScene->Update();
#endif
			ObjectManager::UpdateObjectComponent();

			Box2DBodyManager::ExcuteMoveFunction();

			//�J�����֘A�s��Z�b�g
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

	// �Q�[�����[�v
	while (!loadingComplete && !terminateFlag)
	{
		// �V���Ƀ��b�Z�[�W�������
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
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
			if (nowCount >= oldCount + frameCount)
			{
				//Animator�p�̃J�E���g�X�V
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
				//��O�L���b�`(nullptr�Q�ƂƂ�)
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

int Window::WindowEnd(HINSTANCE hInstance)
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

	//DirectX��������
	DirectX11::D3D_Release();

#ifdef DEBUG_TRUE
	//�R���\�[����ʕ���
	CloseConsoleWindow();
#endif

	UnregisterClass("MAIN_WINDOW", hInstance);

	UnregisterClass("SUB_WINDOW", hInstance);

	//���������[�N�ڍ׌��m
	//(unique_ptr�n�͂��̂��Ɖ�������̂Ō��m����Ă��܂�)
	//_CrtDumpMemoryLeaks();

	return (int)msg.wParam;
}

LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:// �E�B���h�E�j���̃��b�Z�[�W
		PostQuitMessage(0);// �uWM_QUIT�v���b�Z�[�W�𑗂�@���@�A�v���I��
		break;

	case WM_CLOSE:  // �ux�v�{�^���������ꂽ��
	{
		int res = MessageBoxA(NULL, "�I�����܂����H", "�m�F", MB_OKCANCEL);
		if (res == IDOK) {
			terminateFlag = true;
			DestroyWindow(hWnd);  // �uWM_DESTROY�v���b�Z�[�W�𑗂�
		}
	}
	break;

	case WM_MOVING:
	{
		//�J�����֘A�s��Z�b�g
		CameraManager::SetCameraMatrix();

		DirectX11::D3D_StartRender();

		RenderManager::Draw();

#ifdef SFTEXT_TRUE
		SFTextManager::ExcuteDrawString();
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

		//��ʂ̑傫���擾
	case WM_SIZE:
		GetClientRect(hWnd, &windowSize);
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
		//�J�����֘A�s��Z�b�g
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
		// �󂯎�������b�Z�[�W�ɑ΂��ăf�t�H���g�̏��������s
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return LRESULT();
}


