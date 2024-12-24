#pragma once

class EnemyChaser : public Component
{
	SAFE_POINTER(Box2DBody, rb)
	GameObject* target = nullptr;
	float speed = 2.0f;
	float radius = 1000.0f;
	
	void Start() override
	{
		rb = m_this->AddComponent<Box2DBody>();
		rb->SetFixedRotation(true);
		rb->CreateBoxShape();
	}

	void Update() override
	{
		target = ObjectManager::Find("Player");

		if (target != nullptr)
		{
			Vector2 vec = target->transform.position - m_this->transform.position;
			float length = sqrtf(powf(vec.x, 2) + powf(vec.y, 2));
			if (length < radius)
			{
				vec.Normalize();
				vec *= speed;

				rb->AddForceImpulse({ vec.x,0.0f });
			}	
		}
	}

	void OnCollisionEnter(GameObject* other)
	{
		if (other != target) return;

		Box2DBody* targetRb;
		if (other->TryGetComponent<Box2DBody>(&targetRb))
		{
			Vector2 vec = other->transform.position - m_this->transform.position;
			vec.Normalize();
			vec *= 1000.0f;
			targetRb->AddForceImpulse({ vec.x,vec.y });
		}
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle) override
	{
		ImGui::InputFloat("speed", &speed);
		ImGui::InputFloat("radius", &radius);
	}

public:

	void SetTarget(GameObject* _target)
	{
		target = _target;
	}
};
