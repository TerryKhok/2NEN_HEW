#include "YkTestScene.h"
#include "Enemy_type2.h"
#include "MovePlayer.h"

void Enemy_type2::Start()
{
	rb = m_this->GetComponent<Box2DBody>();
}

void Enemy_type2::Update()
{
	if (!fly)
	{
		bool isGround = false;
		Vector2 rayEnd = m_this->transform.position;
		rayEnd.y -= 50.0f;
		if (Box2D::WorldManager::RayCast(m_this->transform.position, rayEnd, F_MAPRAY))
		{
			isGround = true;
		}

		if (!isGround) { return; }
	}
	else
	{
		rb->SetGravityScale(0.0f);
	}

	disCount++;

	// ‚·‚Å‚ÉÅ‘åˆÚ“®‹——£‚Ü‚Åi‚ñ‚Å‚¢‚½‚çÜ‚è•Ô‚·‚æ‚¤‚É‚·‚é
	if (disCount == distance)
	{
		// 0 = ¶, 1 = ‰E
		switch (direction)
		{
		case 0:
			direction = 1;
			disCount = 0;
			break;

		case 1:
			direction = 0;
			disCount = 0;
			break;

		}
	}

	// ˆÚ“®ˆ—
	if (disCount < distance)
	{
		// 0 = ¶, 1 = ‰E
		switch (direction)
		{
		case 0:
			rb->AddForceImpulse({ speed * -1.0f,0.0f });
			break;

		case 1:
			rb->AddForceImpulse({ speed,0.0f });
			break;

		}
	}
}

void Enemy_type2::DrawImGui(ImGuiApp::HandleUI& _handle)
{
	ImGui::InputFloat("speed##Enemy", &speed);
	ImGui::InputFloat("distance##Enemy", &distance);
	ImGui::Checkbox("fly##Enemy", &fly);
}

void Enemy_type2::OnColliderEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<YkTestScene>();
	}
}
