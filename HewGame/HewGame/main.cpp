
#define FPS 60

GameObject* o1;
Component* component;

//�����ɏ������X�V��������������
//============================================================
void MainInit()
{
	o1 = new GameObject();
	o1->AddComponent<Renderer>(L"asset/pic/hart.png");

	/*Vector2 pos = { 100.0f,0.0f };
	o1->SetPosition(pos);
	pos *= -1.0f;
	o2->SetPosition(pos);*/
}
//============================================================

//�����Ƀ��C���X�V��������������
//============================================================
void MainUpdate()
{
	

}
//============================================================

//�����Ƀ��C���`�揈������������
//============================================================
void DrawUpdate()
{

}
//============================================================

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	Window::WindowCreate(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	Window::MainLoop(MainInit, MainUpdate, DrawUpdate, FPS);

	delete o1;

	return Window::WindowEnd(hInstance);
}
