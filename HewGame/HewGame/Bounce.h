#pragma once

class Bounce : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	int inCount = 0;

	const XMFLOAT4 enterColor = { 1.0f,1.0f,1.0f,1.0f };
	const XMFLOAT4 exitColor = { 1.0f,1.0f,1.0f,0.6f };
	float restitutionPower = 0.8f;

	void Start() override
	{
		rend = m_this->GetComponent<Renderer>();
		rend->SetColor(exitColor);

		if (!m_this->TryGetComponent<Box2DBody>(&rb))
		{
			rb = m_this->AddComponent<Box2DBody>();
		}

		if (!rb->IsBullet())
			rb->SetBullet(true);

		if (rb->GetType() != b2_kinematicBody)
			rb->SetType(b2_kinematicBody);
	}

	std::unordered_set<GameObject*> enters;

	void EnterEvent(GameObject* target)
	{
		Box2DBody* rb = nullptr;
		if (target->TryGetComponent<Box2DBody>(&rb))
		{
			enters.insert(target);
			//=======================================================
			rb->SetRestitution(restitutionPower);
			//=======================================================
			rend->SetColor(enterColor);
			inCount++;
		}

		MovePlayer* player = nullptr;
		if (target->TryGetComponent<MovePlayer>(&player))
		{
			player->PushMode(BOUNCE);
		}
	}

	void ExitEvent(GameObject* target)
	{
		Box2DBody* rb = nullptr;
		if (target->TryGetComponent(&rb))
		{
			//=======================================================
			rb->SetRestitution(0.0f);
			//=======================================================
			inCount--;
			if (inCount <= 0)
			{
				rend->SetColor(exitColor);
			}
		}

		MovePlayer* player = nullptr;
		if (target->TryGetComponent<MovePlayer>(&player))
		{
			player->PopMode(BOUNCE);
		}
	}

	int overlapCount = 0;
	void Update() override
	{
		overlapCount++;
		if (overlapCount > 4)
		{
			overlapCount = 0;
			std::unordered_map<GameObject*, b2ShapeId> insideObjects;
			rb->GetOverlapObject(insideObjects);

			for (auto iter = enters.begin(); iter != enters.end();)
			{
				auto it = insideObjects.find(*iter);
				if (it == insideObjects.end())
				{
					ExitEvent(*iter);
					iter = enters.erase(iter);
					continue;
				}
				iter++;
			}
		}
	}

	void OnColliderEnter(GameObject* _other) override
	{
		if (enters.find(_other) != enters.end()) return;

		EnterEvent(_other);
	}

	void OnColliderExit(GameObject* _other) override
	{
		auto iter = enters.find(_other);
		if (iter != enters.end())
		{
			enters.erase(iter);

			ExitEvent(_other);
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUI) override
	{
		ImGui::InputFloat("restitution", &restitutionPower);
	}

	SERIALIZE_COMPONENT_VALUE(restitutionPower)
};

SetReflectionComponent(Bounce)
