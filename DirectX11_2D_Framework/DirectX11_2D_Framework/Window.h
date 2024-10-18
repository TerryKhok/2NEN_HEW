#pragma once

// �}�N����`
#define CLASS_NAME   "DX11"// �E�C���h�E�N���X�̖��O
#define WINDOW_NAME  "DX2DFramework"// �E�B���h�E�̖��O

//�t���[�����[�g
//==============================================================
#define FPS 60
//==============================================================

//������ʂ̑傫��
//================================================================
#define SCREEN_WIDTH (640)	// �E�C���h�E�̕�
#define SCREEN_HEIGHT (480)	// �E�C���h�E�̍���
//================================================================

//��ʂ��f���傫���i���ꂪ���W�n�̑傫���ɂȂ�j
//================================================================
#define PROJECTION_WIDTH (640)
#define PROJECTION_HEIGHT (480)
//================================================================

//Fps��\�����邩�ǂ����i���Ԃ񂠂Ƃœ��I�ɕύX�ł���悤�ɂ���j
//================================================================
#define SHOW_FPS
//================================================================


class Window final
{
	Window() = delete;

	static HWND hWnd;
	static MSG msg;
	static RECT windowRect;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long oldCount;
	static int fpsCounter;		//FPS�v���ϐ�
	static long long oldTick;	//�O��v����
	static long long nowTick;	//����v����
	static long long nowCount;

	//�X���b�h�̏I���t���O
	static std::atomic<bool> terminateFlag;
public:
	//�E�B���h�E����
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//�E�B���h�E������(�V�[���̏�������n��)
	static LRESULT WindowInit(void(*p_mainInitFunc)(void));
	//�E�B���h�E�X�V
	static LRESULT WindowUpdate(/*, void(*p_drawFunc)(void), int fps*/);
	//�񓯊��X�V
	static LRESULT WindowUpdate(std::future<void>& sceneFuture,bool& loading);
	//�E�B���h�E��������
	static int WindowEnd(HINSTANCE hInstance);
private:	
	//�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
