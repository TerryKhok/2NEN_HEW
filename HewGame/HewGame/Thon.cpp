#include "YkTestScene.h"
#include "Thon.h"
#include "MovePlayer.h"

void Thon::Start()
{

}

void Thon::Update()
{
	(this->*pEvent)();
}

void Thon::OnColliderEnter(GameObject* _other)
{
	MovePlayer* player = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&player))
	{
		player->GameOver();
		pEvent = &Thon::EventFunc;
	}
}

void Thon::OnCollisionEnter(GameObject* _other)
{
	MovePlayer* player = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&player))
	{
		player->GameOver();
		pEvent = &Thon::EventFunc;
	}
}
