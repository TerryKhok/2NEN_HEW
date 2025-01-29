#pragma once

class FlickeringRenderColor : public Component
{
	SAFE_POINTER(Renderer, render)

	void Start() override
	{
		render = m_this->GetComponent<Renderer>();
	}

	void Update() override
	{
		// •âŠÔŒW” t (0.0f ` 1.0f ‚Ì”ÍˆÍ)
		float t = time / flickeringTime;

		int nextIndex = (currentIndex + 1) % colors.size();

		// XMFLOAT4 ‚ð XMVECTOR ‚É•ÏŠ·
		XMVECTOR startVec = XMLoadFloat4(&colors[currentIndex]);
		XMVECTOR endVec = XMLoadFloat4(&colors[nextIndex]);

		// ƒŠƒjƒA•âŠÔ
		XMVECTOR resultVec = XMVectorLerp(startVec, endVec, t);

		// Œ‹‰Ê‚ð XMFLOAT4 ‚ÉŠi”[
		XMFLOAT4 result;
		XMStoreFloat4(&result, resultVec);

		render->SetColor(result);

		time += 1.0f / UPDATE_FPS;
		if (time >= flickeringTime)
		{
			time = 0.0f;
			currentIndex = nextIndex;
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override
	{
		ImGui::DragFloat("time##flickeringRenderColor", &flickeringTime);

		if (ImGui::Button("AddColor##flickeringRenderColor"))
		{
			colors.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
		}
		ImGui::SeparatorText("Colors##flickeringRenderColor");

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("FlickeringColors", ImVec2(400, 80), ImGuiChildFlags_Borders);
		{
			int i = 0;
			for (auto iter = colors.begin(); iter != colors.end();)
			{
				ImGui::PushID(i);
				if (colors.size() > 2)
				{
					if (ImGui::Button("erase##fixedRoutePoint"))
					{
						iter = colors.erase(iter);
						if (i >= currentIndex)
						{
							currentIndex = (int)colors.size() - 1;
						}
						ImGui::PopID();
						continue;
					}
					ImGui::SameLine();
				}

				ImGui::ColorEdit4("color##flickeringRenderColor", &iter->x);
				ImGui::PopID();

				i++;
				iter++;
			}

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
	}

	void Serialize(SERIALIZE_OUTPUT& ar) override
	{
		ar(CEREAL_NVP(colors),CEREAL_NVP(flickeringTime));
	}

	void Deserialize(SERIALIZE_INPUT& ar) override
	{
		ar(CEREAL_NVP(colors),CEREAL_NVP(flickeringTime));
	}
private:
	std::vector<DirectX::XMFLOAT4> colors = 
	{
		{ 1.0f,1.0f,1.0f,1.0f },
		{ 1.0f,1.0f,1.0f,1.0f }
	};
	int currentIndex = 0;
	float flickeringTime = 5.0f;
	float time = 0.0f;
};

SetReflectionComponent(FlickeringRenderColor)

class FlickeringTileMapColor : public Component
{
	SAFE_POINTER(TileMap, tilemap)

	void Start() override
	{
		tilemap = m_this->GetComponent<TileMap>();
	}

	void Update() override
	{
		// •âŠÔŒW” t (0.0f ` 1.0f ‚Ì”ÍˆÍ)
		float t = time / flickeringTime;

		int nextIndex = (currentIndex + 1) % colors.size();

		// XMFLOAT4 ‚ð XMVECTOR ‚É•ÏŠ·
		XMVECTOR startVec = XMLoadFloat4(&colors[currentIndex]);
		XMVECTOR endVec = XMLoadFloat4(&colors[nextIndex]);

		// ƒŠƒjƒA•âŠÔ
		XMVECTOR resultVec = XMVectorLerp(startVec, endVec, t);

		// Œ‹‰Ê‚ð XMFLOAT4 ‚ÉŠi”[
		XMFLOAT4 result;
		XMStoreFloat4(&result, resultVec);

		tilemap->SetColor(result);

		time += 1.0f / UPDATE_FPS;
		if (time >= flickeringTime)
		{
			time = 0.0f;
			currentIndex = nextIndex;
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override
	{
		ImGui::DragFloat("time##flickeringtilemapColor", &flickeringTime);

		if (ImGui::Button("AddColor##flickeringtilemapColor"))
		{
			colors.emplace_back(1.0f, 1.0f, 1.0f, 1.0f);
		}
		ImGui::SeparatorText("Colors##flickeringtilemapColor");

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("FlickeringColors", ImVec2(400, 80), ImGuiChildFlags_Borders);
		{
			int i = 0;
			for (auto iter = colors.begin(); iter != colors.end();)
			{
				ImGui::PushID(i);
				if (colors.size() > 2)
				{
					if (ImGui::Button("erase##fixedRoutePoint"))
					{
						iter = colors.erase(iter);
						if (i >= currentIndex)
						{
							currentIndex = (int)colors.size() - 1;
						}
						ImGui::PopID();
						continue;
					}
					ImGui::SameLine();
				}

				ImGui::ColorEdit4("color##flickeringtilemapColor", &iter->x);
				ImGui::PopID();

				i++;
				iter++;
			}

			ImGui::EndChild();
		}
		ImGui::PopStyleVar();
	}

	void Serialize(SERIALIZE_OUTPUT& ar) override
	{
		ar(CEREAL_NVP(colors), CEREAL_NVP(flickeringTime));
	}

	void Deserialize(SERIALIZE_INPUT& ar) override
	{
		ar(CEREAL_NVP(colors), CEREAL_NVP(flickeringTime));
	}
private:
	std::vector<DirectX::XMFLOAT4> colors =
	{
		{ 1.0f,1.0f,1.0f,1.0f },
		{ 1.0f,1.0f,1.0f,1.0f }
	};
	int currentIndex = 0;
	float flickeringTime = 5.0f;
	float time = 0.0f;
};

SetReflectionComponent(FlickeringTileMapColor)
