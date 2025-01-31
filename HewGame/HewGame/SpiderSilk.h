#pragma once

class SpiderSilk : public Component
{
	SAFE_POINTER(GameObject,target)

	void Proceed() override
	{
		target = ObjectManager::Find(targetName);
	}

	void Update() override
	{
		Vector2 ceilingPos;
		const auto& rayStart = target->transform.position;
		Vector2 rayEnd = rayStart;
		rayEnd.y += silkLength;
		if (Box2D::WorldManager::RayCast(rayStart, rayEnd, ceilingPos, F_MAPRAY))
		{
			auto& transform = m_this->transform;

			Vector2 dis = ceilingPos - rayStart;
			transform.position = rayStart;
			transform.position += dis / 2;
			transform.scale.y = sqrt(dis.x * dis.x + dis.y * dis.y) / DEFAULT_OBJECT_SIZE;
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle) override
	{
		static char str[128] = {};
		memset(str, '\0', strlen(str));
		memcpy(str, targetName.c_str(), targetName.size());
		if (ImGui::InputText("targetName##SpiderSilk", str, sizeof(str), ImGuiInputTextFlags_EnterReturnsTrue) && str[0] != '\0')
		{
			targetName = str;
		}

		ImGui::InputFloat("length##spiderSilk", &silkLength);
	}

private:
	std::string targetName;
	float silkLength = 500.0f;

	SERIALIZE_COMPONENT_VALUE(targetName, silkLength)
};

SetReflectionComponent(SpiderSilk)
