#pragma once

class Bounce : public Component
{
	SAFE_POINTER(Renderer, rend)
	SAFE_POINTER(Box2DBody, rb)

	int inCount = 0;

	const XMFLOAT4 enterColor = { 1.0f,0.0f,0.0f,0.5f };
	const XMFLOAT4 exitColor = { 0.8f,0.2f,0.2f,0.4f };
	float restitutionPower = 0.8f;

	void Start() override
	{
		rend = m_this->GetComponent<Renderer>();
		rend->SetColor(enterColor);

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

	void OnColliderEnter(GameObject* _other) override
	{
		if (enters.find(_other) != enters.end()) return;

		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent<Box2DBody>(&rb))
		{
			enters.insert(_other);
			rb->SetRestitution(restitutionPower);
			rend->SetColor(enterColor);
			inCount++;
		}
	}

	void OnColliderExit(GameObject* _other) override
	{
		Box2DBody* rb = nullptr;
		if (_other->TryGetComponent(&rb))
		{
			auto iter = enters.find(_other);
			if (iter != enters.end()) enters.erase(iter);

			rb->SetRestitution(0.0f);
			inCount--;
			if (inCount <= 0)
			{
				rend->SetColor(exitColor);
			}
		}
	}

	std::unordered_map<GameObject*, b2ShapeId> enterObjects;
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

					rb->SetRestitution(0.0f);
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
	}

	void DrawImGui(ImGuiApp::HandleUI& _handleUI) override
	{
		ImGui::InputFloat("restitution", &restitutionPower);
	}

	SERIALIZE_COMPONENT_VALUE(restitutionPower)
};

SetReflectionComponent(Bounce)
