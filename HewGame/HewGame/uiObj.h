#pragma once

class uiObj :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);
	float cnt     = 0.0f; // 経過時間
	float trigger = 3.0f; // 起動時間

	float speed = 3.0f;    // 移動スピード
	float mvCnt = 0.0f;    // 移動時間
	float turnDis = 50.0f; // 移動距離
	 
	bool rbType  = false; // リジットボディのタイプ
	bool touchFg = false; // 接触フラグ
	bool turnFg  = false;

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
			switch (rbType)
			{
			case false:
				rb->SetType(b2_dynamicBody);
				rbType = true;
				break;
			}
		}

		switch (touchFg)
		{
		case true:
			switch (turnFg)
			{
			case false:
				mvCnt++;
				rb->SetVelocityX(speed);
				break;

			case true:
				mvCnt++;
				rb->SetVelocityX(speed * -1.0f);
				break;
			}
			
			if (mvCnt == turnDis)
			{
				mvCnt = 0;
				switch (turnFg)
				{
				case false:
					turnFg = true;
					break;

				case true:
					turnFg = false;
					break;
				}
			}

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
		ImGui::InputFloat("mvCnt##uiObj",   &mvCnt);
		ImGui::InputFloat("turnDis##uiObj", &turnDis);

	}
};

SetReflectionComponent(uiObj);