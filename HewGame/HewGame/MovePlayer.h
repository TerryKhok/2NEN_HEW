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

		if (rb->GetShapeCount() == 0)
			rb->CreateCapsuleShape();
		rb->SetGravityScale(10.0f);
		rb->SetBullet(true);
	}

public:
	bool inWindow = false;
private:
	bool jumping = false;
	int jump_count = 0;
	bool left_moving = false;
	bool right_moving = false;
	int left_count = 0;
	int right_count = 0;
	int movement = 0;

	void Update()
	{
		auto& input = Input::Get();

		bool isGround = false;
		Vector2 rayEnd = m_this->transform.position;
		rayEnd.y -= 50.0f;
		if (Box2D::WorldManager::RayCast(m_this->transform.position, rayEnd, F_MAPRAY))
		{
			isGround = true;
		}

		if ((input.KeyPress(VK_D) || input.LeftAnalogStick().x > 0.1f))
		{
			right_moving = true;
			rb->SetVelocityX({ 0 + (float)right_count });
			input.SetVibration(1, 1);
		}

		if ((input.KeyPress(VK_A) || input.LeftAnalogStick().x < -0.1f))
		{
			left_moving = true;
			rb->SetVelocityX({ 0 - (float)left_count });
			input.SetVibration(1, 1);
		}

		/*if (input.KeyTrigger(VK_D))
		{
			right_moving = true;
			movement = 1;
		}

		if (input.KeyTrigger(VK_A))
		{
			left_moving = true;
			movement = 2;

		}*/

		//switch (movement)//¶ˆÚ“®‚Ì—Dæ‡ˆÊ‚ª‰EˆÚ“®‚æ‚è‚‚¢–â‘è‚ð’¼‚·
		//{
		//case 1:
		//{
		//	if ((input.KeyPress(VK_D))&& right_moving)
		//	{
		//		rb->SetVelocityX({ 0 + (float)right_count });
		//		input.SetVibration(1);
		//	}
		//	break;
		//}
		//case 2:
		//{
		//	if ((input.KeyPress(VK_A))&& left_moving)
		//	{
		//		rb->SetVelocityX({ 0 - (float)left_count });
		//	}
		//	break;
		//}
		//}

		if ((input.KeyTrigger(VK_W) || input.ButtonTrigger(XINPUT_A)) && isGround && !jumping && !inWindow)
		{
			jump_count = 0;
			jumping = true;
			rb->AddForce({ 0,10 });
		}

		if ((input.KeyPress(VK_W) || input.ButtonPress(XINPUT_A)) && jumping && !inWindow)
		{
			rb->AddForceImpulse({ 0,20.0f - (float)jump_count });
		}


		// fall gravety
		float top = rb->GetVelocity().y;
		if (top <= 0)
		{
			rb->AddForceImpulse({ 0,rb->GetGravityScale() * -0.25f });
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
			if (left_count < 50)
			{
				left_count++;
			}
			if (input.KeyRelease(VK_A) || (input.LeftAnalogStick().x > -0.1f && input.IsConnectController()))
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
			if (input.KeyRelease(VK_D) || (input.LeftAnalogStick().x < 0.1f && input.IsConnectController()))
			{
				right_count = 0;
				right_moving = false;
			}
		}
	}
};

SetReflectionComponent(MovePlayer)
