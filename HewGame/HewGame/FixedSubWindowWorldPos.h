#pragma once

class FixedSubWindowWorldPos : public Component
{
	SAFE_POINTER(SubWindow,subWindow)

	void Start() override
	{
		subWindow = m_this->GetComponent<SubWindow>();
	}


	void Update() override
	{
		auto& pos = m_this->transform.position;
		Vector2 windowPos = GetWindowPosition(subWindow->GeWndHandle());
		if (abs(windowPos.x - pos.x) > 5.0f || abs(windowPos.y - pos.y) > 5.0f)
		{
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
	}

	int count = 0;
};

SetReflectionComponent(FixedSubWindowWorldPos)