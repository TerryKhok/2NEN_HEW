#pragma once

class SelectTrigger : public Component
{
	SAFE_POINTER(Renderer,render)
	//SAFE_POINTER(GameObject,target)

	void Start() override
	{
		render = m_this->GetComponent<Renderer>();
	}

	void Update() override
	{
		auto target = ObjectManager::Find(targetName);
		if (target != nullptr)
		{
			auto& pos = m_this->transform.position;
			auto scale = m_this->transform.scale;
			scale *= HALF_OBJECT_SIZE;
			auto& tPos = target->transform.position;
			auto tScale = target->transform.scale;
			tScale *= HALF_OBJECT_SIZE;

			bool hit = (((pos.x + scale.x) > (tPos.x - tScale.x)) &&
				((pos.x - scale.x) < (tPos.x + tScale.x)) &&
				((pos.y + scale.y) > (tPos.y - tScale.y)) &&
				((pos.y - scale.y) < (tPos.y + tScale.y)));

			if (hit)
			{
				EnterEvent();
			}
			else
			{
				ExitEvent();
			}
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override
	{
		static char str[128] = {};
		memset(str, '\0', strlen(str));
		memcpy(str, targetName.c_str(), targetName.size());
		if (ImGui::InputText("##SelectTrigger", str, sizeof(str), ImGuiInputTextFlags_EnterReturnsTrue) && str[0] != '\0')
		{
			targetName = str;
		}
		FunctionRegistry::DrawPickFunction("Event##SelectTrigger", funcName);
	}
public:
	void EnterEvent()
	{
		render->SetColor({ 0.8f,0.0f,0.8f,1.0f });
		if (Input::Get().KeyTrigger(VK_F))
		{
			FunctionRegistry::Get().callFunction(funcName);
		}
	}

	void ExitEvent()
	{
		render->SetColor({ 1.0f,1.0f,1.0f,1.0f });
	}
private:
	std::string targetName = "null";
	std::string funcName;

	SERIALIZE_COMPONENT_VALUE(targetName, funcName)
};

SetReflectionComponent(SelectTrigger)
