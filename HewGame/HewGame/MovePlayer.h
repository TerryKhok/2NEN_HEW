#pragma once

class MovePlayer : public Component
{
	SAFE_POINTER(Box2DBody, rb)
	
	void Start()
	{
		if (!m_this->TryGetComponent<Box2DBody>(&rb))
		{
			rb = m_this->AddComponent<Box2DBody>();
		}
		rb->SetFilter(F_PLAYER);
		rb->SetFixedRotation(true);
		rb->CreateCapsuleShape();
	}

	bool jumping = false;
	int count = 0;

	void Update()
	{
		bool isGround = false;
		Vector2 rayEnd = m_this->transform.position;
		rayEnd.y -= 50.0f;
		if (Box2D::WorldManager::RayCast(m_this->transform.position, rayEnd))
		{
			isGround = true;
		}

		if (Input::Get().KeyPress(VK_E))
		{
			DeleteObject(m_this);
		}

		if (Input::Get().KeyPress(VK_RIGHT))
		{
			rb->SetVelocityX(15.0f);
		}
		if (Input::Get().KeyPress(VK_LEFT))
		{
			rb->SetVelocityX(-15.0f);
		}

		if (Input::Get().KeyTrigger(VK_UP) && isGround && !jumping)
		{
			rb->AddForceImpule({ 0.0f,50.0f });
		}

		if (jumping)
		{
			count++;
			if (count > 20)
			{
				count = 0;
				jumping = false;
			}
		}
	}
};
