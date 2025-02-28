#pragma once

class Timer : public Component
{
	void Update() override
	{
		static constexpr float frameTime = 1.0f / UPDATE_FPS;
		time += frameTime;
		if (time > stayTime)
		{
			DeleteObject(m_this);
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle) override {
		ImGui::InputFloat("StayTime##Timer", &stayTime);
	}

	float time = 0.0f;
	float stayTime = 1.0f;

	void Serialize(SERIALIZE_OUTPUT& ar) override {
		ar(CEREAL_NVP(stayTime));
	}
	void Deserialize(SERIALIZE_INPUT& ar) override {
		ar(CEREAL_NVP(stayTime));
	}
};

SetReflectionComponent(Timer)
