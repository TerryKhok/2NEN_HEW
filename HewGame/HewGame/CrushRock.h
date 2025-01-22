#pragma once

#include "MovePlayer.h"

class CrushRock : public Component
{
	void OnCollisionEnter(GameObject* _ohter) override
	{
		MovePlayer* player = nullptr;
		if (_ohter->TryGetComponent<MovePlayer>(&player))
		{
			player->GameOver();
		}

		DeleteObject(m_this);
	}
};

SetReflectionComponent(CrushRock)
