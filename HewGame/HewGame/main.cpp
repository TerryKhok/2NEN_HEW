#include "Sample_SceneLoad.h"
#include "Scene_ProtoType.h"
#include "SeigoTestScene.h"
#include "TitleScene.h"
#include "YkTestScene.h"


SetReflectionComponent(BreakScaffold)
SetReflectionComponent(Fog)
SetReflectionComponent(Rock)
SetReflectionComponent(Thon)
SetReflectionScene(YkTestScene)

//シーンの登録、レイヤーの設定
//============================================================
void MainInit()
{	
	Box2DBodyManager::DisableCollisionFilter(F_WINDOW, F_TERRAIN);
	Box2DBodyManager::DisableCollisionFilter(F_WINDOW, F_WINDOW);
	Box2DBodyManager::DisableCollisionFilter(F_WINDOW, F_OBSTACLE);
	Box2DBodyManager::DisableCollisionFilter(F_WINDOW, F_PERWINDOW);
	Box2DBodyManager::DisableCollisionFilter(F_MAPRAY, F_WINDOW);
	Box2DBodyManager::DisableCollisionFilter(F_MAPRAY, F_PLAYER);
	Box2DBodyManager::DisableCollisionFilter(F_MAPRAY, F_PERWINDOW);
	Box2DBodyManager::OnlyCollisionFilter(F_PERWINDOW, F_ONLYOBSTACLE);
	Box2DBodyManager::OnlyCollisionFilter(F_ONLYOBSTACLE, F_OBSTACLE);
	Box2DBodyManager::EnableCollisionFilter(F_ONLYOBSTACLE, F_PERWINDOW);
}
//============================================================


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Window::WindowMainCreate(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	Window::WindowInit(MainInit);
	Window::WindowUpdate();

	return Window::WindowEnd();
}
