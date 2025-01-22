#pragma once

class LowGravity : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	int inCount = 0;

	const XMFLOAT4 enterColor = { 1.0f,1.0f,1.0f,1.0f };
	const XMFLOAT4 exitColor = { 1.0f,1.0f,1.0f,0.2f };
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

	void OnColliderEnter(GameObject* _other) override
	{
		if (enters.find(_other) != enters.end()) return;

		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent<Box2DBody>(&rb))
		{
			enters.insert(_other);
			/*float top = rb->GetVelocity().y;
			if (top <= 0)
			{
				rb->SetVelocityY({ rb->GetVelocity().y * 0.2f });
			}*/
			//rb->SetVelocityY({ rb->GetVelocity().y * 0.2f});
			rb->SetGravityScale(rb->GetGravityScale() / lowPower);
			rend->SetColor(enterColor);
			inCount++;
		}

		MovePlayer* player = nullptr;
		if (_other->TryGetComponent<MovePlayer>(&player))
		{
			player->SetMode(LOW_GRAVITY);
		}
	}

	void OnColliderExit(GameObject* _other) override
	{
		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent(&rb))
		{
			auto iter = enters.find(_other);
			if (iter != enters.end()) enters.erase(iter);

			rb->SetGravityScale(rb->GetGravityScale() * lowPower);
			inCount--;
			if (inCount <= 0)
			{
				rend->SetColor(exitColor);
			}
		}

		MovePlayer* player = nullptr;
		if (_other->TryGetComponent<MovePlayer>(&player))
		{
			player->BackMode();
		}
	}

	/*std::unordered_map<GameObject*, b2ShapeId> enterObjects;
	Vector2 enterPos;

	void OnWindowEnter(HWND _hWnd) override
	{
		rb->GetOverlapObject(enterObjects);
		enterPos = m_this->transform.position;
	}

	void OnWindowExit(HWND _hWnd) override
	{
		Vector2 pos = GetWindowPosition(_hWnd);
		rb->SetPosition(pos);

		std::unordered_map<GameObject*, b2ShapeId> exitObjects;
		rb->GetOverlapObject(exitObjects);

		for (auto object : enterObjects)
		{
			auto iter = exitObjects.find(object.first);
			if (iter == exitObjects.end())
			{
				Box2DBody* rb = nullptr;
				if (object.first->TryGetComponent(&rb))
				{
					auto iterator = enters.find(object.first);
					if (iterator != enters.end()) enters.erase(iterator);

					rb->SetGravityScale(rb->GetGravityScale() * lowPower);
					inCount--;
					if (inCount <= 0)
					{
						rend->SetColor(exitColor);
					}
				}
			}
		}
		for (auto object : exitObjects)
		{
			auto iter = enterObjects.find(object.first);
			if (iter == enterObjects.end())
			{
				Box2DBody* rb = nullptr;
				if (object.first->TryGetComponent<Box2DBody>(&rb))
				{
					rb->SetAwake(true);
				}
			}
		}

		enterObjects.clear();
	}*/
};

SetReflectionComponent(LowGravity)