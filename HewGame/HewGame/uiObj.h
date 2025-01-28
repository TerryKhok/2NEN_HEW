#pragma once

class uiObj :public Component
{
private:
	SAFE_POINTER(Box2DBody, rb);
	float cnt     = 0.0f; // �o�ߎ���
	float trigger = 3.0f; // �N������

	float speed = 3.0f;    // �ړ��X�s�[�h
	float mvCnt = 0.0f;    // �ړ�����
	float turnDis = 50.0f; // �ړ�����
	 
	bool rbType  = false; // ���W�b�g�{�f�B�̃^�C�v
	bool touchFg = false; // �ڐG�t���O
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