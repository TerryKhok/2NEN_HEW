#pragma once

class Obj :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);

public:
	virtual void Start() { rb = m_this->GetComponent<Box2DBody>(); }

	virtual void Update() = 0;

	virtual void DrawImGui(ImGuiApp::HandleUI& _handle) = 0;
};