#pragma once

class uiObj :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);
	float cnt     = 0.0f; // �o�ߎ���
	float trigger = 3.0f; // �N������

	float speed = 3.0f; // �ړ��X�s�[�h
	float mvCnt = 0.0f; // �ړ�����

	int   change  = 0; // ���W�b�g�{�f�B�̃^�C�v

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