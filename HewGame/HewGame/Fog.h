#pragma once

class Fog : public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);

	void Start();
	void Update();
};