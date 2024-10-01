#include "Window.h"
const char* relativePath(const char* fullPath) {
#ifdef _WIN32
	const char separator = '\\'; // Windows uses backslashes
#else
	const char separator = '/';  // Unix-like systems use slashes
#endif
	const char* relative = strrchr(fullPath, separator);
	return relative ? relative + 1 : fullPath; // Return the file name if found, else return the full path
}

void setConsoleTextColor(unsigned int color)
{
#ifdef DEBUG_TRUE
	static const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, color);
#endif
}

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

HWND Window::hWnd;
MSG Window::msg;
RECT Window::windowRect;
LARGE_INTEGER Window::liWork;
long long Window::frequency;
long long Window::oldCount;
int Window::fpsCounter = 0;						//FPS�v���ϐ�
long long Window::oldTick = GetTickCount64();	//�O��v����
long long Window::nowTick = oldTick;				//����v����
long long Window::nowCount = oldCount;

std::atomic<bool> Window::terminateFlag(false);

LRESULT Window::WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

#ifdef DEBUG_TRUE
	//�R���\�[����ʋN��
	CreateConsoleWindow();
#endif

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

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
	wc.lpszClassName = CLASS_NAME;
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	// �E�B���h�E�̏����܂Ƃ߂�
	hWnd = CreateWindowEx(0,	// �g���E�B���h�E�X�^�C��
		CLASS_NAME,				// �E�B���h�E�N���X�̖��O
		WINDOW_NAME,			// �E�B���h�E�̖��O
		WS_OVERLAPPEDWINDOW,	// �E�B���h�E�X�^�C��
		CW_USEDEFAULT,			// �E�B���h�E�̍���w���W
		CW_USEDEFAULT,			// �E�B���h�E�̍���x���W 
		SCREEN_WIDTH,			// �E�B���h�E�̕�
		SCREEN_HEIGHT,			// �E�B���h�E�̍���
		NULL,					// �e�E�B���h�E�̃n���h��
		NULL,					// ���j���[�n���h���܂��͎q�E�B���h�EID
		hInstance,				// �C���X�^���X�n���h��
		NULL);					// �E�B���h�E�쐬�f�[�^


	//�E�B���h�E�̃T�C�Y���C��
	RECT rc1, rc2;
	GetWindowRect(hWnd, &rc1);
	GetClientRect(hWnd, &rc2);
	int sx = SCREEN_WIDTH;
	int sy = SCREEN_HEIGHT;
	sx += ((rc1.right - rc1.left) - (rc2.right - rc2.left));
	sy += ((rc1.bottom - rc1.top) - (rc2.bottom - rc2.top));
	SetWindowPos(hWnd, NULL, 0, 0, sx, sy, (SWP_NOZORDER |
		SWP_NOOWNERZORDER | SWP_NOMOVE));

	// �w�肳�ꂽ�E�B���h�E�̕\����Ԃ�ݒ�(�E�B���h�E��\��)
	ShowWindow(hWnd, nCmdShow);
	// �E�B���h�E�̏�Ԃ𒼂��ɔ��f(�E�B���h�E�̃N���C�A���g�̈���X�V)
	UpdateWindow(hWnd);

	//FPS�Œ�p�ϐ�
	QueryPerformanceFrequency(&liWork);
	frequency = liWork.QuadPart;
	QueryPerformanceCounter(&liWork);
	oldCount = liWork.QuadPart;

	//DirectX����
	DirectX11::D3D_Create(hWnd);

	return LRESULT();
}

LRESULT Window::WindowInit(void(*p_sceneInitFunc)(void))
{
	//����������
	//=================================================
	RenderManager::Init();
	p_sceneInitFunc();
	SceneManager::Init();
	//=================================================

	return LRESULT();
}

LRESULT Window::WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/)
{
	// �Q�[�����[�v
	while (1)
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
			if (nowCount >= oldCount + frequency / FPS)
			{
				Input::Get().Update();

				SceneManager::m_currentScene->Update();

				ObjectManager::UpdateObjectComponent();

				DirectX11::D3D_StartRender();

				RenderManager::Draw();

				DirectX11::D3D_FinishRender();

				oldCount = nowCount;
				fpsCounter++;
				nowTick = GetTickCount64();

				if (nowTick >= oldTick + 1000)
				{
				/*	char str[32];
					wsprintfA(str, "FPS = %d", fpsCounter);
					SetWindowTextA(hWnd, str);*/

					fpsCounter = 0;
					oldTick = nowTick;
				}
			}
		}
	}

	return LRESULT();
}

LRESULT Window::WindowUpdate(std::future<void>& sceneFuture,bool& loading)
{
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
			if (nowCount >= oldCount + frequency / FPS)
			{
				Input::Get().Update();

				SceneManager::m_currentScene->Update();

				ObjectManager::UpdateObjectComponent();

				DirectX11::D3D_StartRender();

				RenderManager::Draw();

				DirectX11::D3D_FinishRender();

				oldCount = nowCount;
				fpsCounter++;
				nowTick = GetTickCount64();

				if (nowTick >= oldTick + 1000)
				{
					/*	char str[32];
						wsprintfA(str, "FPS = %d", fpsCounter);
						SetWindowTextA(hWnd, str);*/

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
		LOG("Scene loading stopped!");
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
	ObjectManager::Uninit();

	//DirectX��������
	DirectX11::D3D_Release();


#ifdef DEBUG_TRUE
	//�R���\�[����ʕ���
	CloseConsoleWindow();
#endif

	UnregisterClass(CLASS_NAME, hInstance);

	//���������[�N�ڍ׌��m
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

	case WM_KEYDOWN: //�L�[���͂����������b�Z�[�W
		if (LOWORD(wParam) == VK_ESCAPE) { //���͂��ꂽ�L�[��ESCAPE�Ȃ�
			PostMessage(hWnd, WM_CLOSE, wParam, lParam);//�uWM_CLOSE�v�𑗂�
		}
		break;

	case WM_MOUSEMOVE:
	{
		GetCursorPos(&Input::mousePoint);
		ScreenToClient(hWnd, &Input::mousePoint);
		static const LONG HALF_SCREEN_WIDTH  = SCREEN_WIDTH / 2;
		static const LONG HALF_SCREEN_HEIGHT = SCREEN_HEIGHT / 2;
		Input::mousePoint.x = Input::mousePoint.x * SCREEN_WIDTH / windowRect.right - HALF_SCREEN_WIDTH;
		Input::mousePoint.y = Input::mousePoint.y * SCREEN_WIDTH / windowRect.bottom - HALF_SCREEN_HEIGHT;
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
		//��ʂ̑傫���擾
	case WM_SIZE:
		GetClientRect(hWnd, &windowRect);
		break;
	default:
		// �󂯎�������b�Z�[�W�ɑ΂��ăf�t�H���g�̏��������s
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return 0;
}


