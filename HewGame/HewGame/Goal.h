#pragma once

class Goal : public Component
{
	std::string sceneName = "GameClearScene";
	int waitCount = 60;
	int count = 0;
	void(Goal::*pEvent)() = &Goal::VoidFunc;

	void VoidFunc() {}

	void EventFunc()
	{
		count++;
		if (count > waitCount)
		{
			SceneManager::LoadScene(sceneName);
		}
	}

	void Update() override
	{
		(this->*pEvent)();
	}

	void OnColliderEnter(GameObject* _ohter) override
	{
		if (_ohter->ExistComponent<MovePlayer>())
		{
			LOG("touch goal!!");
			pEvent = &Goal::EventFunc;
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override
	{
		static char str[128] = {};
		
		memset(str, '\0', sizeof(str) / sizeof(char));
		memcpy(str, sceneName.c_str(), sceneName.size());
		if (ImGui::InputText("SceneName##Goal", str, sizeof(str), ImGuiInputTextFlags_EnterReturnsTrue) && str[0] != '\0')
		{
			sceneName = str;
		}

		ImGui::InputInt("waitCount", &waitCount);
	}

	SERIALIZE_COMPONENT_VALUE(sceneName, waitCount)
};

SetReflectionComponent(Goal);
