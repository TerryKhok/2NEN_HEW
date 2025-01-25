#pragma once

class uiObj :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);
	float cnt     = 0.0f; // 経過時間
	float trigger = 3.0f; // 起動時間

	float speed = 3.0f; // 移動スピード
	float mvCnt = 0.0f; // 移動時間

	int   change  = 0; // リジットボディのタイプ

	bool touchFg = false;

public:
	void Start()
	{
		rb = m_this->GetComponent<Box2DBody>();
		rb->SetType(b2_staticBody);
	}

	void Update()
	{
		if (cnt < trigger)
		{
			cnt += 1.0f / UPDATE_FPS;
		}
		else if (cnt >= trigger)
		{
			switch (change)
			{
			case 0:
				rb->SetType(b2_dynamicBody);
				change = 1;
				break;
			}
		}

		switch (touchFg)
		{
		case true:
			rb->SetVelocityX(speed);

			break;
		}
		
				
	}

	void OnCollisionEnter(GameObject* _other)
	{
		touchFg = true;
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle)
	{
		ImGui::InputFloat("cnt##uiObj",     &cnt);
		ImGui::InputFloat("trigger##uiObj", &trigger);
		ImGui::InputFloat("speed##uiObj",   &speed);
		ImGui::InputFloat("mvCnt##uiObj", &mvCnt);
	}
};

SetReflectionComponent(uiObj);