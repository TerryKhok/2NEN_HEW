#include "Sample_SceneLoad.h"
#include "Sample_Box2D.h"
//#include "../../DirectX11_2D_Framework/DirectX11_2D_Framework/HelloBox2d.h"

//シーンの登録
//============================================================
void SceneInit()
{
	//一番上のシーンが初めにロードされる
	SceneManager::RegisterScene<Box2D_SampleScene>();

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
