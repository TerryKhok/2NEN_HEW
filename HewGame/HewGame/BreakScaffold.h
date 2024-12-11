#pragma once

class  BreakScaffold : public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);

	bool touchFg = false;

	void Start();
	void Update();
	void OnColliderEnter(GameObject* _other) override;
};