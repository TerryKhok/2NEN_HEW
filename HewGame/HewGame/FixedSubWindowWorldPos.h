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
		if (windowPos != pos)
		{
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
	}
};

SetReflectionComponent(FixedSubWindowWorldPos)