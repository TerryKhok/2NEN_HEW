#pragma once

class BreakWall : public Component
{
	SAFE_POINTER(Box2DBody,rb)

	void Start() override
	{
		rb = m_this->GetComponent<Box2DBody>();
	}

	void OnCollisionEnter(GameObject* _other) override
	{
		Box2DBody* box2d = nullptr;
		if (_other->TryGetComponent(&box2d))
		{
			if (targetFilter == box2d->GetFilter())
			{
				DeleteObject(m_this);
			}
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override
	{
		if (ImGui::Button("<>##breakWall"))
		{
			ImGui::OpenPopup("changeFilterbreakWall");
		}
		ImGui::SetItemTooltip("change filer");
		ImGui::SameLine();
		ImGui::Text("Filter : %s", magic_enum::enum_name(targetFilter).data());

		if (ImGui::BeginPopup("changeFilterbreakWall"))
		{
			static long long numFilter = magic_enum::enum_count<FILTER>();
			for (int i = 0; i < numFilter; i++)
			{
				FILTER filter = (FILTER)pow(2, i);
				bool same = targetFilter == filter;
				if (ImGui::Selectable(magic_enum::enum_name(filter).data(), &same))
				{
					targetFilter = filter;
				}
			}
			ImGui::EndPopup();
		}
	}

	FILTER targetFilter = FILTER::FILTER_01;

	SERIALIZE_COMPONENT_VALUE(targetFilter)
};

SetReflectionComponent(BreakWall)
