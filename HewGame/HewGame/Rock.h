#pragma once

class Rock : public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
	void OnCollisionEnter(GameObject* _other) override;
};