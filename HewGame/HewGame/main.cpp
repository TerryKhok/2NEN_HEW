#include "Sample_SceneLoad.h"
#include "Scene_ProtoType.h"


//�V�[���̓o�^�A���C���[�̐ݒ�
//============================================================
void MainInit()
{	
	//��ԏ�̃V�[�������߂Ƀ��[�h�����
	//SceneManager::RegisterScene<AnimationBulider_Scene>();
	SceneManager::RegisterScene<Scene_ProtoType>();
	SceneManager::RegisterScene<SampleScene_Title>();
	SceneManager::RegisterScene<SampleScene_Loading>();
	SceneManager::RegisterScene<SampleScene_Box2D>();
	SceneManager::RegisterScene<SampleScene_Animation>();

	//LAYER_01��LAYER_02���Փ˂��Ȃ��悤�ɂ���
	Box2DBodyManager::DisableLayerCollision(F_WINDOW, F_TERRAIN);
	Box2DBodyManager::DisableLayerCollision(F_WINDOW, F_WINDOW);
	Box2DBodyManager::DisableLayerCollision(F_WINDOW, F_OBSTACLE);
	Box2DBodyManager::DisableLayerCollision(F_PERMEATION, F_OBSTACLE);
	Box2DBodyManager::OnlyCollisionFilter(F_ONLYOBSTACLE, F_OBSTACLE);
}
//============================================================


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Window::WindowCreate(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	Window::WindowInit(MainInit);
	Window::WindowUpdate();

	return Window::WindowEnd();
}
