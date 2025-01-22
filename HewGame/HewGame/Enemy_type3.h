#pragma once

class Enemy_type3 :public Component
{
private:

	float speed     = 1.0f;
	float distance  = 150.0f;
	float disCount  = 0.0f;
	int   direction = 0; // 0 = è„, 1 = â∫

	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
	void DrawImGui(ImGuiApp::HandleUI& _handle);
	void OnColliderEnter(GameObject* _other) override;

};

SetReflectionComponent(Enemy_type3);