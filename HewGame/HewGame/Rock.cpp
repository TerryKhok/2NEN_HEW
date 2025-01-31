#include "Rock.h"
#include "MovePlayer.h"


void Rock::Start()
{
	rb = m_this->GetComponent<Box2DBody>();
}

void Rock::Update()
{
	(this->*pEvent)();
}

void Rock::OnColliderEnter(GameObject* _other)
{
	MovePlayer* player = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&player))
	{
		player->GameOver();
		pEvent = &Rock::EventFunc;
	}
}

void Rock::OnCollisionEnter(GameObject* _other)
{
	b2Vec2 vec = rb->GetVelocity();
	if (vec.x < 2 && vec.y < 2) return;

	MovePlayer* player = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&player))
	{
		player->GameOver();
		pEvent = &Rock::EventFunc;
	}
}
