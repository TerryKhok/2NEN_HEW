#pragma once

class Enemy_type3 :public Component
{
private:

	float speed = 1.0f;
	float distance = 150.0f;
	int   disCount = 0;
	int   direction = 0; // 0 = ��, 1 = ��

	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
	void OnColliderEnter(GameObject* _other) override;

};

SetReflectionComponent(Enemy_type3);