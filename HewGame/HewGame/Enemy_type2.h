#pragma once

class Enemy_type2 :public Component
{
private:

	float speed     = 1.0f;
	float distance  = 150.0f;
	int   disCount  = 0;
	int   direction = 0; // 0 = ç∂, 1 = âE

	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
	void OnColliderEnter(GameObject* _other) override;
	
};

SetReflectionComponent(Enemy_type2);