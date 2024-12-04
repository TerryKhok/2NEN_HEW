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
		rb->SetGravityScale(10.0f);
	}

	bool jumping = false;
	bool left_moving = false;
	bool right_moving = false;
	int jump_count = 0;
	int left_count = 0;
	int right_count = 0;
	int movement = 0;

	void Update()
	{
		bool isGround = false;
		Vector2 rayEnd = m_this->transform.position;
		rayEnd.y -= 50.0f;
		if (Box2D::WorldManager::RayCast(m_this->transform.position, rayEnd,F_PLAYER_RAY))
		{
			isGround = true;
		}

		if (Input::Get().KeyPress(VK_E))
		{
			DeleteObject(m_this);
		}

		if (Input::Get().KeyTrigger(VK_RIGHT))
		{
			right_moving = true;
			movement = 1;
		}

		if (Input::Get().KeyTrigger(VK_LEFT))
		{
			left_moving = true;
			movement = 2;

		}

		switch (movement)//¶ˆÚ“®‚Ì—Dæ‡ˆÊ‚ª‰EˆÚ“®‚æ‚è‚‚¢–â‘è‚ð’¼‚·
		{
		case 1:
		{
			if (Input::Get().KeyPress(VK_RIGHT) && right_moving)
			{
				rb->SetVelocityX({ 0 + (float)right_count });
			}
			break;
		}
		case 2:
		{
			if (Input::Get().KeyPress(VK_LEFT) && left_moving)
			{
				rb->SetVelocityX({ 0 - (float)left_count });
			}
			break;
		}
		}

		if (Input::Get().KeyTrigger(VK_UP) && isGround && !jumping)
		{
			jumping = true;
			rb->AddForce({ 0,10 });
		}

		if (Input::Get().KeyPress(VK_UP) && jumping)
		{
			rb->AddForceImpule({ 0,20.0f - (float)jump_count });
		}


		// fall gravety
		float top = rb->GetVelocity().y;
		if (top <= 0)
		{
			rb->AddForceImpule({ 0,rb->GetGravityScale() * -0.25f });
		}

		if (jumping)
		{
			jump_count++;
			if (jump_count > 10)
			{
				jump_count = 0;
				jumping = false;
			}
		}

		if (left_moving)
		{
			if (left_count < 30)
			{
				left_count++;
			}
			if (Input::Get().KeyRelease(VK_LEFT))
			{
				left_count = 0;
				left_moving = false;
			}
		}

		if (right_moving)
		{
			if (right_count < 30)
			{
				right_count++;
			}
			if (Input::Get().KeyRelease(VK_RIGHT))
			{
				right_count = 0;
				right_moving = false;
			}
		}
	}
};
