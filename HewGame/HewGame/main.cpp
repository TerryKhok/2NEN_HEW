#include "Sample_SceneLoad.h"

//#include "../../DirectX11_2D_Framework/DirectX11_2D_Framework/HelloBox2d.h"

//�V�[���̓o�^�A���C���[�̐ݒ�
//============================================================
void MainInit()
{	
	//��ԏ�̃V�[�������߂Ƀ��[�h�����
	SceneManager::RegisterScene<SampleScene01>();
	SceneManager::RegisterScene<SampleScene02>();
	SceneManager::RegisterScene<LoadScene>();
	SceneManager::RegisterScene<Box2D_SampleScene>();

	//LAYER_01��LAYER_02���Փ˂��Ȃ��悤�ɂ���
	Box2DBodyManager::DisableLayerCollision(LAYER_01, LAYER_02);
}
//============================================================


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Window::WindowCreate(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	Window::WindowInit(MainInit);
	Window::WindowUpdate();

	return Window::WindowEnd(hInstance);
}
