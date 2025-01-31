#pragma once

// �}�N����`
#define WINDOW_NAME  "�� -�E�B���h�E-"// �E�B���h�E�̖��O

//�t���[�����[�g
//================================================================
#define UPDATE_FPS 60
//================================================================

//================================================================
#define WORLD_FPS 120
//================================================================

//������ʂ̑傫��
//================================================================
#define SCREEN_WIDTH (1152)	// �E�C���h�E�̕�
#define SCREEN_HEIGHT (864)	// �E�C���h�E�̍���
//#define SCREEN_WIDTH (960)	// �E�C���h�E�̕�
//#define SCREEN_HEIGHT (720)	// �E�C���h�E�̍���
//================================================================

//������ʂ̑傫��
//================================================================
#define SUB_SCREEN_WIDTH (320)	// �E�C���h�E�̕�
#define SUB_SCREEN_HEIGHT (320)	// �E�C���h�E�̍���
//================================================================

//��ʂ��f���傫���i���ꂪ���W�n�̑傫���ɂȂ�j
//================================================================
#define PROJECTION_WIDTH (320)
#define PROJECTION_HEIGHT (240)
//================================================================

//��ʂɑ΂��镨�̂̑傫��
//================================================================
constexpr float PROJECTION_WINDOW_WIDTH = SCREEN_WIDTH / 3.0f;
constexpr float PROJECTION_WINDOW_HEIGHT = SCREEN_HEIGHT / 3.0f;
//================================================================

//��ʂ̑傫���ƍ��W�̑傫���̔�
constexpr float DISPALY_ASPECT_WIDTH = static_cast<float>(SCREEN_WIDTH) / PROJECTION_WIDTH;
constexpr float DISPALY_ASPECT_HEIGHT = static_cast<float>(SCREEN_HEIGHT) / PROJECTION_HEIGHT;

//��ʂ̑傫���ƍ��W�̑傫���̔�
constexpr float PROJECTION_ASPECT_WIDTH = static_cast<float>(PROJECTION_WIDTH) / PROJECTION_WINDOW_WIDTH;
constexpr float PROJECTION_ASPECT_HEIGHT = static_cast<float>(PROJECTION_WIDTH) / PROJECTION_WINDOW_WIDTH;

//���C���E�B���h�E�𓮂����Ȃ��悤�ɂ���
//================================================================
#define MAINWINDOW_LOCK
//================================================================


#ifdef DEBUG_TRUE
//��Ƀ��C���̏�ɃT�u������
#define SUBWINDOW_IS_TOP
#endif

//�E�B���h�E���}�E�X�œ������Ȃ�����
//================================================================
#define SUBWINDOW_MOVELOCK
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

#ifdef DEBUG_TRUE
#define WM_PAUSE_DEBUG (WM_USER + 6)
#define WM_RESUME_DEBUG (WM_USER + 7)
#endif

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

#ifdef MAINLOOP__MALUTITHREAD
	//���C�������̏I���t���O
	static std::atomic<bool> mainLoopRun;
#endif

	//�񓯊����[�h�̏I���t���O
	static std::atomic<bool> terminateFlag;

	//�E�B���h�E�̃n���h���ɑΉ������I�u�W�F�N�g�̖��O
	static std::unordered_map<HWND, const std::string&> m_hwndObjNames;
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
	static const HWND& GetMainHWnd();
	//�Q�[�����ꎞ��~����
	static void PauseGame();
	//�Q�[�����ĊJ����
	static void ResumeGame();
	//�Q�[����~�����ǂ���
	static const bool IsPause();
#ifdef DEBUG_TRUE
	static const bool IsPauseDebug();
#endif
public:
	static thread_local HWND(*pWindowSubCreate)(const std::string&, std::string, int, int, Vector2);
private:	
	static HWND WindowSubCreate(const std::string& _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//�T�u�E�B���h�E�쐬
	static HWND WindowSubCreateAsync(const std::string& _objName, std::string _windowName = "SUB", int _width = SUB_SCREEN_WIDTH, int _height = SUB_SCREEN_HEIGHT, Vector2 _pos = { 0,0 });
	//�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//�T�u�E�B���h�E�p�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//�E�B���h�E�𓮂����邩��ݒ肷��
	static void SetWindowMovable(HWND hwnd, bool movable);
};
