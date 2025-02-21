#include "YkTestScene.h"
#include "Enemy_type3.h"
#include "MovePlayer.h"

void Enemy_type3::Start()
{
	rb = m_this->GetComponent<Box2DBody>();
}

void Enemy_type3::Update()
{
	disCount++;

	// すでに最大移動距離まで進んでいたら折り返すようにする
	if (disCount == distance)
	{
		// 0 = 上, 1 = 下
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

	// 移動処理
	if (disCount < distance)
	{
		// 0 = 上, 1 = 下
		switch (direction)
		{
		case 0:
			rb->AddForceImpulse({ 0.0f, speed });
			break;

		case 1:
			rb->AddForceImpulse({ 0.0f, speed * -1.0f });
			break;

		}
	}
}

void Enemy_type3::DrawImGui(ImGuiApp::HandleUI& _handle)
{
	ImGui::InputFloat("speed##Enemy", &speed);
	ImGui::InputFloat("distance##Enemy", &distance);
}

void Enemy_type3::OnColliderEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<YkTestScene>();
	}
}