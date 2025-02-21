#pragma once

class uiObj :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);
	//アニメーション用
	SAFE_POINTER(Animator, ani);
	float cnt     = 0.0f; // 経過時間
	float trigger = 3.0f; // 起動時間

	float speed = 3.0f;              // 移動スピード
	float turnDis = 200.0f;           // 移動距離
	float mvCnt = turnDis / 2.0f;    // 移動時間
	 
	bool rbType  = false; // リジットボディのタイプ
	bool touchFg = false; // 接触フラグ
	bool turnFg  = false;

public:
	void Start()
	{
		rb = m_this->GetComponent<Box2DBody>();
		ani = m_this->GetComponent<Animator>();
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

				//アニメーションを反転させる
				ani->Reverse(true);
				break;

			case true:
				mvCnt++;
				rb->SetVelocityX(speed * -1.0f);

				//アニメーションの反転を戻す
				ani->Reverse(false);
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

	SERIALIZE_COMPONENT_VALUE(trigger, speed, turnDis, mvCnt)
};

SetReflectionComponent(uiObj);