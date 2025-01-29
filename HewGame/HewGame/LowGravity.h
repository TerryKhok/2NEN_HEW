#pragma once

class LowGravity : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	int inCount = 0;

	const XMFLOAT4 enterColor = { 1.0f,1.0f,1.0f,1.0f };
	const XMFLOAT4 exitColor = { 1.0f,1.0f,1.0f,0.6f };
	const float lowPower = 50.0f;

	void Start() override
	{
		rend = m_this->GetComponent<Renderer>();
		rend->SetColor(exitColor);

		if (!m_this->TryGetComponent<Box2DBody>(&rb))
		{
			rb = m_this->AddComponent<Box2DBody>();
		}
	}

	std::unordered_set<GameObject*> enters;

	void EnterEvent(GameObject* target)
	{
		Box2DBody* rb = nullptr;
		if (target->TryGetComponent<Box2DBody>(&rb))
		{
			enters.insert(target);
			//=======================================================
			rb->SetGravityScale(rb->GetGravityScale() / lowPower);
			//=======================================================
			rend->SetColor(enterColor);
			inCount++;
		}

		MovePlayer* player = nullptr;
		if (target->TryGetComponent<MovePlayer>(&player))
		{
			player->PushMode(LOW_GRAVITY);
		}
	}

	void ExitEvent(GameObject* target)
	{
		Box2DBody* rb = nullptr;
		if (target->TryGetComponent(&rb))
		{
			//=======================================================
			rb->SetGravityScale(rb->GetGravityScale() * lowPower);
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
			player->PopMode(LOW_GRAVITY);
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
};

SetReflectionComponent(LowGravity)