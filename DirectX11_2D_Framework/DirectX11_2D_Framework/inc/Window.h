#pragma once

// �}�N����`
#define WINDOW_NAME  "DX2DFramework"// �E�B���h�E�̖��O

//�t���[�����[�g
//================================================================
#define UPDATE_FPS 60
//================================================================

//================================================================
#define WORLD_FPS 240
//================================================================

//������ʂ̑傫��
//================================================================
#define SCREEN_WIDTH (960)	// �E�C���h�E�̕�
#define SCREEN_HEIGHT (720)	// �E�C���h�E�̍���
//================================================================

//������ʂ̑傫��
//================================================================
#define SUB_SCREEN_WIDTH (320)	// �E�C���h�E�̕�
#define SUB_SCREEN_HEIGHT (320)	// �E�C���h�E�̍���
//================================================================

//��ʂ��f���傫���i���ꂪ���W�n�̑傫���ɂȂ�j
//================================================================
#define PROJECTION_WIDTH (640)
#define PROJECTION_HEIGHT (480)
//================================================================

//��ʂ̑傫���ƍ��W�̑傫���̔�
constexpr float DISPALY_ASPECT_WIDTH = static_cast<float>(SCREEN_WIDTH) / PROJECTION_WIDTH;
constexpr float DISPALY_ASPECT_HEIGHT = static_cast<float>(SCREEN_HEIGHT) / PROJECTION_HEIGHT;

//���C���E�B���h�E�𓮂����Ȃ��悤�ɂ���
//================================================================
#define MAINWINDOW_LOCK
//================================================================

//��Ƀ��C���̏�ɃT�u������
//================================================================
#define SUBWINDOW_IS_TOP
//================================================================

//�E�B���h�E�̈ʒu�ɃJ������u��
//================================================================
#define CAMERA_ON_WINDOW
//================================================================


#define WM_CREATE_NEW_WINDOW (WM_USER + 1)
#define WM_DELETE_WINDOW (WM_USER + 2)
#define WM_ADJUST_Z_ORDER (WM_USER + 3)
#define WM_PAUSE_GAME (WM_USER + 4)
#define WM_RESUME_GAME (WM_USER + 5)

//�E�B���h�E�̈ʒu���擾����
Vector2 GetWindowPosition(HWND _hWnd);
//�E�B���h�E�̈ʒu��ύX����
void SetWindowPosition(HWND _hWnd, Vector2 pos);

class Window final
{
public:
	//���j�^�[�̉𑜓x����
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
	static int worldFpsounter;	//FPS�v���ϐ�
	static int updateFpsCounter;		//FPS�v���ϐ�
	static long long oldTick;	//�O��v����
	static long long nowTick;	//����v����
	static long long nowCount;

#ifdef MAINLOOP__MALUTITHREAD
	//���C�������̏I���t���O
	static std::atomic<bool> mainLoopRun;
#endif

	//�񓯊����[�h�̏I���t���O
	static std::atomic<bool> terminateFlag;

	//�E�B���h�E�̃n���h���ɑΉ������I�u�W�F�N�g�̖��O
	static std::unordered_map<HWND, std::string> m_hwndObjNames;
public:
	//�E�B���h�E����
	static LRESULT WindowMainCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);

	//�T�u�E�B���h�E�폜
	static void WindowSubRelease(HWND hWnd);
	//�V�[�����[�f�B���O�J�n
	static void WindowSubLoadingBegin();
	//�V�[�����[�f�B���O�I��(���ׂẴE�B���h�E��\������)
	static void WindowSubLoadingEnd();
	//���݂̃E�B���h�E��ێ����Ă���I�u�W�F�N�g��j������
	static void WindowSubHide();
	static void WindowSubHide(HWND _hWnd);

	//�E�B���h�E������(�V�[���̏�������n��)
	static LRESULT WindowInit(void(*p_mainInitFunc)(void));
	//�E�B���h�E�X�V
	static LRESULT WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/);
	//�񓯊��X�V
	static LRESULT WindowUpdate(std::future<void>& sceneFuture, bool& loading);
	//�E�B���h�E��������
	static int WindowEnd();
	//���C���̃E�B���h�E�n���h���擾
	static const HWND& GetMainHwnd(){return mainHwnd;}
public:
	static thread_local HWND(*pWindowSubCreate)(std::string, std::string, int, int, Vector2);
private:	
	static HWND WindowSubCreate(std::string _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//�T�u�E�B���h�E�쐬
	static HWND WindowSubCreateAsync(std::string _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//�T�u�E�B���h�E�p�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//�E�B���h�E�𓮂����邩��ݒ肷��
	static void SetWindowMovable(HWND hwnd, bool movable);
};
