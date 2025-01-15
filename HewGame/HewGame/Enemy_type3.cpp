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

	// ‚·‚Å‚ÉÅ‘åˆÚ“®‹——£‚Ü‚Åi‚ñ‚Å‚¢‚½‚çÜ‚è•Ô‚·‚æ‚¤‚É‚·‚é
	if (disCount == distance)
	{
		// 0 = ã, 1 = ‰º
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
		// 0 = ã, 1 = ‰º
		switch (direction)
		{
		case 0:
			m_this->transform.position.y += speed;
			break;

		case 1:
			m_this->transform.position.y -= speed;
			break;

		}
	}
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