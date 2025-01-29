#pragma once

class RockGenerator : public Component
{
	void Start() override
	{

	}

	void Update() override
	{
		count++;
		if (count > createCount)
		{
			count = 0;
			auto rock = LoadObject(rockPath);
			Box2DBody* rb = nullptr;
			if (rock->TryGetComponent<Box2DBody>(&rb))
			{
				rb->SetAwake(true);
				rb->SetPosition(m_this->transform.position);
				rb->SetVelocity({ shootVec.x,shootVec.y });
			}
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle) override
	{
#ifdef DEBUG_TRUE
		if (ImGui::Button("Link##rockObject"))
		{
			std::string str = m_this->GetName() + "RockObject";
			_handle.SetUploadFile(str, [&](GameObject* obj, std::filesystem::path path)
				{
					if (obj == nullptr || obj != m_this) return;

					auto rockGenerator = obj->GetComponent<RockGenerator>();
					if (rockGenerator != nullptr)
					{
						rockGenerator->rockPath = path.string();
					}
				}, { ".json"});
		}
		ImGui::SameLine();
		ImGui::Text("path  : %s", rockPath.c_str());

		float time = static_cast<float>(createCount / UPDATE_FPS);
		if (ImGui::DragFloat("count", &time, 0.1f, 0.0f))
		{
			createCount = static_cast<int>(time * UPDATE_FPS);
		}

		ImGui::InputFloat2("velocity", shootVec.data());
#endif
	}

private:
	std::string rockPath;
	int createCount = 300;
	int count = 0;
	Vector2 shootVec;

	SERIALIZE_COMPONENT_VALUE(rockPath, createCount, shootVec)
};

SetReflectionComponent(RockGenerator)
