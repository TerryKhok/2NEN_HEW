#pragma once
#include "Obj.h"

// ���Ɉړ�����I�u�W�F�N�g
class ObjMW :public Obj
{
private:
	int   diR   = 0;     // �i�s����(�O�����A�P���E)
	float disT  = 10.0f; // �ړ�����
	float speed = 0.0f;  // �ړ����x
	float cur   = 0.0f;  // ���݂̈ړ�����

	void Serialize(cereal::JSONOutputArchive& ar) override {
		if constexpr (3 <= 7) {
			::cereal::make_optional_nvp(ar, "diR", diR); ::cereal::make_optional_nvp(ar, "disT", disT); ::cereal::make_optional_nvp(ar, "speed", speed);
		}
		else {
			ar(diR, disT, speed);
		}
	} void Deserialize(cereal::JSONInputArchive& ar) override {
		if constexpr (3 <= 7) {
			::cereal::make_optional_nvp(ar, "diR", diR); ::cereal::make_optional_nvp(ar, "disT", disT); ::cereal::make_optional_nvp(ar, "speed", speed);
		}
		else {
			ar(diR, disT, speed);
		}
	}


public:
	
	void Update()
	{
		cur++;

		// �ړ�����
		if (diR == 0)
		{
			m_this->transform.position.x -= speed;
		}
		else if (diR == 1)
		{
			m_this->transform.position.x += speed;
		}

		// �����؂�ւ�����
		if (cur == disT)
		{
			cur = 0; // ���݂̈ړ����Ԃ����Z�b�g

			switch (diR)
			{
			case 0:
				diR = 1;
				break;

			case 1:
				diR = 0;
				break;

			}
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle)
	{
		ImGui::InputInt("diR##ObjMW",     &diR);
		ImGui::InputFloat("disT##ObjMW",  &disT);
		ImGui::InputFloat("speed##ObjMW", &speed);
	}
};