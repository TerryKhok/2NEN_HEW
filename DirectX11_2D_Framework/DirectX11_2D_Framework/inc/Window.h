#pragma once

// �}�N����`
#define WINDOW_NAME  "DX2DFramework"// �E�B���h�E�̖��O

//�t���[�����[�g
//==============================================================
#define FPS 60
//==============================================================

//������ʂ̑傫��
//================================================================
#define SCREEN_WIDTH (960)	// �E�C���h�E�̕�
#define SCREEN_HEIGHT (720)	// �E�C���h�E�̍���
//================================================================

//������ʂ̑傫��
//================================================================
#define SUB_SCREEN_WIDTH (640)	// �E�C���h�E�̕�
#define SUB_SCREEN_HEIGHT (640)	// �E�C���h�E�̍���
//================================================================

//��ʂ��f���傫���i���ꂪ���W�n�̑傫���ɂȂ�j
//================================================================
#define PROJECTION_WIDTH (640)
#define PROJECTION_HEIGHT (480)
//================================================================

	//��ʂ̑傫���ƍ��W�̑傫���̔�
constexpr float DISPALY_ASPECT_WIDTH = static_cast<float>(SCREEN_WIDTH) / PROJECTION_WIDTH;
constexpr float DISPALY_ASPECT_HEIGHT = static_cast<float>(SCREEN_HEIGHT) / PROJECTION_HEIGHT;

//Fps��\�����邩�ǂ����i���Ԃ񂠂Ƃœ��I�ɕύX�ł���悤�ɂ���j
//================================================================
#define SHOW_FPS
//================================================================


class Window final
{
public:
	enum HANDLE_TYPE
	{
		MAIN,
		HANDLE_MAX
	};

private:
	Window() = delete;

	static HWND mainHwnd;
	static MSG msg;
	static RECT windowSize;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long oldCount;
	static int fpsCounter;		//FPS�v���ϐ�
	static long long oldTick;	//�O��v����
	static long long nowTick;	//����v����
	static long long nowCount;

#ifdef MAINLOOP__MALUTITHREAD
	//���C�������̏I���t���O
	static std::atomic<bool> mainLoopRun;
#endif

	//�񓯊����[�h�̏I���t���O
	static std::atomic<bool> terminateFlag;

	//�E�B���h�E�𓮂����Ă��邩�̃t���O
	static std::atomic<bool> windowSizeMove;
public:
	//�E�B���h�E����
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//�T�u�E�B���h�E�쐬
	static LRESULT WindowSubCreate(HINSTANCE hInstance,int nCmdShow,const char* _windowName = "SUB");
	//�E�B���h�E������(�V�[���̏�������n��)
	static LRESULT WindowInit(void(*p_mainInitFunc)(void));
	//�E�B���h�E�X�V
	static LRESULT WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/);
	//�񓯊��X�V
	static LRESULT WindowUpdate(std::future<void>& sceneFuture, bool& loading);
	//�E�B���h�E��������
	static int WindowEnd(HINSTANCE hInstance);
	//���C���̃E�B���h�E�n���h���擾
	static const HWND& GetMainHwnd(){return mainHwnd;}
private:	
	//�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//�T�u�E�B���h�E�p�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProcSub(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
