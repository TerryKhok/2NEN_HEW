#pragma once

//�t���[�����[�g
//==============================================================
#define FPS 60
//==============================================================

// �}�N����`
#define CLASS_NAME   "DX11"// �E�C���h�E�N���X�̖��O
#define WINDOW_NAME  "DX2DFramework"// �E�B���h�E�̖��O

#define SCREEN_WIDTH (640)	// �E�C���h�E�̕�
#define SCREEN_HEIGHT (480)	// �E�C���h�E�̍���

const char* relativePath(const char* fullPath);
void setConsoleTextColor(unsigned int color);

#ifdef DEBUG_TRUE

#define LOG(format, ...) printf_s("\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__)
#define LOG_ERROR(format, ...) setConsoleTextColor(4);printf_s("\n-ERROR!!-\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__);setConsoleTextColor(7)
#define LOG_WARNING(format, ...) setConsoleTextColor(6);printf_s("\n-WARNING!-\n<file:%s,line:%d>",relativePath(__FILE__),__LINE__);printf_s(format,__VA_ARGS__);setConsoleTextColor(7)

#define LOG_NL printf_s("\n")

#else
#define LOG(format, ...)  
#define LOG_ERROR(format, ...)  
#define LOG_WARNING(format, ...)  

#define LOG_NL
#endif

class Window final
{
	Window() = delete;

	static HWND hWnd;
	static MSG msg;
	static RECT windowRect;
	static LARGE_INTEGER liWork; 
	static long long frequency;
	static long long oldCount;
	static int fpsCounter;						//FPS�v���ϐ�
	static long long oldTick;	//�O��v����
	static long long nowTick;				//����v����
	static long long nowCount;

	//flag to signal termination
	static std::atomic<bool> terminateFlag;
public:
	//�E�B���h�E����
	static LRESULT WindowCreate(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	//�E�B���h�E������(�V�[���̏�������n��)
	static LRESULT WindowInit(void(*p_sceneInitFunc)(void));
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
