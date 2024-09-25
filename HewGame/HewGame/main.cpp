#include "SampleScene.h"

//シーンの登録(main.cppを変更しないとシーンの変更も反映されないので修正する)
// 別のファイルに書くか？
//============================================================
void SceneInit()
{
	//一番上のシーンが初めにロードされる
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
