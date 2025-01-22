#pragma once

class Enemy :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
	void OnColliderEnter(GameObject* _other) override;
};