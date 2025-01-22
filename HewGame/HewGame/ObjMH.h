#pragma once
#include "Obj.h"

// 縦に移動するオブジェクト
class ObjMH	:public Obj
{
private:
	int   diR   = 0;     // 進行方向(０＝下、１＝上)
	float disT  = 10.0f; // 移動距離
	float speed = 0.0f;  // 移動速度
	float cur   = 0.0f;  // 現在の移動時間

	void Serialize(cereal::JSONOutputArchive& ar) override {
		if constexpr (3 <= 7) {
			::cereal::make_optional_nvp(ar, "diR", diR) ; ::cereal::make_optional_nvp(ar, "disT", disT); ::cereal::make_optional_nvp(ar, "speed", speed);
		}
		else {
			ar(diR, disT, speed);
		}
	} void Deserialize(cereal::JSONInputArchive& ar) override {
		if constexpr (3 <= 7) {
			::cereal::make_optional_nvp(ar, "diR", diR) ; ::cereal::make_optional_nvp(ar, "disT", disT); ::cereal::make_optional_nvp(ar, "speed", speed);
		}
		else {
			ar(diR, disT, speed);
		}
	}


public:

	void Update()
	{
		cur++;

		// 移動処理
		if (diR == 0)
		{
			m_this->transform.position.y -= speed;
		}
		else if (diR == 1)
		{
			m_this->transform.position.y += speed;
		}
		
		// 方向切り替え処理
		if (cur == disT)
		{
			cur = 0; // 現在の移動時間をリセット

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
		ImGui::InputInt("diR##ObjMH",     &diR);
		ImGui::InputFloat("disT##ObjMH",  &disT);
		ImGui::InputFloat("speed##ObjMH", &speed);
	}
};