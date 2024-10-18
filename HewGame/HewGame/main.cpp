#include "Sample_SceneLoad.h"

//#include "../../DirectX11_2D_Framework/DirectX11_2D_Framework/HelloBox2d.h"

//シーンの登録、レイヤーの設定
//============================================================
void MainInit()
{	
	//一番上のシーンが初めにロードされる
	SceneManager::RegisterScene<SampleScene01>();
	SceneManager::RegisterScene<SampleScene02>();
	SceneManager::RegisterScene<LoadScene>();
	SceneManager::RegisterScene<Box2D_SampleScene>();

	//LAYER_01とLAYER_02が衝突しないようにする
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
