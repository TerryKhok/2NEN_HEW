#pragma once

#include "GameManager.h"

class SelectTrigger : public Component
{
	SAFE_POINTER(Renderer,render)
	//SAFE_POINTER(GameObject,target)

	void Start() override
	{
		render = m_this->GetComponent<Renderer>();
		render->SetColor(beforeColor);
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
		ImGui::ColorEdit4("beforeColor##selectTrigger", &beforeColor.x);
		ImGui::ColorEdit4("selectColor##selectTrigger", &selectColor.x);
	}
public:
	void EnterEvent()
	{
		render->SetColor(selectColor);
		if (Input::Get().KeyTrigger(VK_F) || Input::Get().ButtonTrigger(XINPUT_A))
		{
			FunctionRegistry::Get().callFunction(funcName);
		}
	}

	void ExitEvent()
	{
		render->SetColor(beforeColor);
	}
private:
	std::string targetName = "null";
	std::string funcName;
	DirectX::XMFLOAT4 beforeColor = { 1.0f,1.0f,1.0f,1.0f };
	DirectX::XMFLOAT4 selectColor = { 1.0f,1.0f,1.0f,1.0f };

	SERIALIZE_COMPONENT_VALUE(targetName, funcName, beforeColor, selectColor)
};

SetReflectionComponent(SelectTrigger)

class RotationObject : public Component
{
	void Start() override
	{
		startAngle = m_this->transform.angle.z.Get();
	}

	void Update() override
	{
		float t = time / rotationTime;

		auto& angleZ = m_this->transform.angle.z;
		angleZ.Set(startAngle + rotationAngle * t);

		time += 1.0f / UPDATE_FPS;
		if (time >= rotationTime)
		{
			m_this->RemoveComponent<RotationObject>();
		}
	}

	float time = 0.0f;
	double startAngle = 0.0;
public:
	float rotationTime = 1.25f;
	double rotationAngle = Math::PI;
};

class TitleTrigger : public Component
{
	void Update() override
	{
		if (!loading)
		{
			m_this->SetActive(false);
			auto ui = Instantiate("rollArrow", L"asset/pic/rollArrow.png");
			ui->transform.position = m_this->transform.position;
			ui->transform.scale = { 10.0f,10.0f };
			auto rotate = ui->AddComponent<RotationObject>();
			rotate->rotationAngle = -Math::PI2;
			rotate->rotationTime = 1.0f;

			loading = true;
			SceneManager::LoadingScene("SelectScene");

			m_this->SetActive(true);
			DeleteObject(ui);
		}

		if (Input::Get().KeyTrigger(VK_RETURN) || Input::Get().ButtonTrigger(XINPUT_A))
		{
			SceneManager::ChangeScene();
		}
	}

	bool loading = false;
};

SetReflectionComponent(TitleTrigger)

class ClearTrigger : public Component
{
	void Proceed() override
	{
		if (GameManager::currentStage == 12)
		{
			auto nextStage = ObjectManager::Find("NextStage");
			if (nextStage != nullptr)
			{
				DeleteObject(nextStage);
			}
		}
	}
};

SetReflectionComponent(ClearTrigger)

class MoveVerticalPoint : public Component
{
	void Start() override
	{
		startPos = m_this->transform.position;
	}

	void Update() override
	{
		float t = time / moveTime;
		Vector2 targetPos;
		targetPos.Lerp(t, startPos, endPos);
		m_this->transform.position = targetPos;
		time += (1.0f / UPDATE_FPS);
		if (time >= moveTime)
		{
			m_this->RemoveComponent<MoveVerticalPoint>();
		}
	}

	Vector2 startPos;
	float time = 0.0f;
public:
	Vector2 endPos;
	float moveTime = 1.25f;
};


