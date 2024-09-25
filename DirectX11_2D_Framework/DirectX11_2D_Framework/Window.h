#pragma once

// �}�N����`
#define CLASS_NAME   "DX11"// �E�C���h�E�N���X�̖��O
#define WINDOW_NAME  "DX2DFramework"// �E�B���h�E�̖��O

#define SCREEN_WIDTH (640)	// �E�C���h�E�̕�
#define SCREEN_HEIGHT (480)	// �E�C���h�E�̍���

const char* relativePath(const char* fullPath);

#ifdef DEBUG_TRUE
#define LOG(format, ...) printf_s("\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__)

#else
#define LOG(format, ...)  

#endif

class Window
{
	Window() = delete;

	static HWND hWnd;
	static MSG msg;
	static RECT windowRect;
public:
	//�E�B���h�E����
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//���C�����[�v(�X�V�����������������ɓn��)
	static LRESULT MainLoop(void(*p_initFunc)(void), void(*p_updateFunc)(void), void(*p_drawFunc)(void), int fps);
	//�E�B���h�E��������
	static int WindowEnd(HINSTANCE hInstance);
private:
	//�R�[���o�b�N�֐�
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
