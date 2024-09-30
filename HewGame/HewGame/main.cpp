#include "SampleScene.h"

//�V�[���̓o�^(main.cpp��ύX���Ȃ��ƃV�[���̕ύX�����f����Ȃ��̂ŏC������)
// �ʂ̃t�@�C���ɏ������H
//============================================================
void SceneInit()
{
	//��ԏ�̃V�[�������߂Ƀ��[�h�����
	SceneManager::RegisterScene<SampleScene01>();
	SceneManager::RegisterScene<SampleScene02>();
	SceneManager::RegisterScene<LoadScene>();
}
//============================================================


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Window::WindowCreate(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	Window::WindowInit(SceneInit);
	Window::WindowUpdate();

	return Window::WindowEnd(hInstance);
}
