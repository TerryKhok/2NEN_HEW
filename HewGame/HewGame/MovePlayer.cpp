#include "MovePlayer.h"

void MovePlayer::ChangeState(PLAYER_STATE _state)
{
	if (state != nullptr)
		state->End(mode, this);

	switch (_state)
	{
	case PLAYER_IDLE:
		state.reset(new PlayerIdle());
		break;
	case PLAYER_WALK:
		state.reset(new PlayerWalk());
		break;
	case PLAYER_JUMP:
		state.reset(new PlayerJump());
		break;
	case PLAYER_FLOAT:
		state.reset(new PlayerFloat());
		break;
	case PLAYER_DAMAGE:
		state.reset(new PlayerDamage());
		break;
	case PLAYER_GOAL:
		state.reset(new PlayerGoal());
		break;
	}

	state->Start(mode, this);
}

void MovePlayer::Update()
{
	auto& input = Input::Get();

	state->Update(mode, this);

	SetModeTexture(NORMAL);

	Vector2 rayStart = m_this->transform.position;
	if (inFloat)
		rayStart.y -= 50.0f;
	else
		rayStart.y += 50.0f;

	Vector2 rayEnd = rayStart;
	if (inFloat)
		rayEnd.y += 60.0f;
	else
		rayEnd.y -= 60.0f;

	isGround = false;
	if (Box2D::WorldManager::RayCastShape(rayStart, rayEnd, rb, F_MAPRAY)) {
		isGround = true;
		airCount = 0;
	}
	else {
		if (airCount > 10)
		{
			if (!inAir && !jumping)
			{
				landCount = 0;
				landing = false;
				ChangeState(PLAYER_FLOAT);
			}
			inAir = true;
		}
		else
			airCount++;
	}

	bool rightHit = false;
	rayStart = m_this->transform.position;
	rayEnd = rayStart;
	rayEnd.x += 10.0f;
	auto& size = m_this->transform.scale;
	float rad = (float)m_this->transform.angle.z.Get();

	auto& pos = m_this->transform.position;
	//地面ポリゴンを作る。 b2MakeBox()ヘルパー関数を使い、地面ポリゴンを箱型にする。箱の中心は親ボディの原点である。
	b2Polygon polygonBox =
		b2MakeOffsetBox(size.x / 4.0f, size.y / 3.5f, { 0.0f,0.0f }, b2MakeRot(rad));
	if (Box2D::WorldManager::RayCastPolygon(rayStart, rayEnd, rb, polygonBox, F_MAPRAY)) {
		rightHit = true;
	}

	constexpr double turnSpeed = 0.1;

	auto& angleZ = m_this->transform.angle.z;
	double radZ = angleZ.Get();
	if (inFloat)
	{
		if (radZ == Math::PI)
		{
			//angleZ.Set(Math::PI);
		}
		else if (radZ > Math::PI)
		{
			angleZ.Set(radZ -= turnSpeed);

			if (radZ <= Math::PI)
			{
				angleZ.Set(Math::PI);
			}
		}
		else if (radZ > 0.0)
		{
			angleZ.Set(radZ += turnSpeed);

			if (radZ >= Math::PI)
			{
				angleZ.Set(Math::PI);
			}
		}

		else
		{
			angleZ.Set(reverse ? 0.001 : Math::PI2 - 0.001);
		}

		anim->Reverse(!reverse);
	}
	else
	{
		if (radZ > Math::PI)
		{
			if (radZ < Math::PI2)
			{
				angleZ.Set(radZ += turnSpeed);
			}
			else
			{
				angleZ.Set(0.0);
			}
		}
		else if (radZ < Math::PI)
		{
			if (radZ > 0.0)
			{
				angleZ.Set(radZ -= turnSpeed);
			}
			else
			{
				angleZ.Set(0.0);
			}
		}
		else
		{
			angleZ.Set(reverse ? Math::PI - 0.001 : Math::PI + 0.001);
		}

		anim->Reverse(reverse);
	}

	render->SetOffset({ 0.0f,15.0f * (float)cos(angleZ.Get()) });

	if ((input.KeyPress(VK_D) || input.LeftAnalogStick().x > 0.1f))
	{
		//if(isGround&&!sound.IsPlaying()){sound.PlayWaveSound(L"asset/sound/se/SFX_Walk01.wav", &waveData, false); }
		if (move_count == 1)
		{
			landCount = 0;
			landing = false;

			reverse = true;

			if (!jumping && !inAir)
				ChangeState(PLAYER_WALK);
		}
		if (move_count < 30)
		{
			move_count++;
		}
		if (!rightHit)
			rb->SetVelocityX((float)move_count);
		else
			rb->SetVelocityX(0.0f);

		PlayWalkSFX();
	}
	else if (move_count > 0)
	{
		if (input.KeyRelease(VK_D)) {
			move_count = 0;
			walk_count = 0;
		}
		else
			move_count--;
		if (move_count == 0 && !jumping && !inAir)
		{
			ChangeState(PLAYER_IDLE);
		}
		/*if (input.KeyRelease(VK_D) || (input.LeftAnalogStick().x < 0.1f && input.IsConnectController()))
		{
			move_count = 0;
			ChangeState(PLAYER_IDLE);
		}*/
	}

	bool leftHit = false;
	rayStart = m_this->transform.position;
	rayEnd = rayStart;
	rayEnd.x -= 10.0f;
	if (Box2D::WorldManager::RayCastPolygon(rayStart, rayEnd, rb, polygonBox, F_MAPRAY)) {
		leftHit = true;
	}
	if ((input.KeyPress(VK_A) || input.LeftAnalogStick().x < -0.1f))
	{
		if (move_count == -1)
		{
			landCount = 0;
			landing = false;

			reverse = false;
			if (!jumping && !inAir)
				ChangeState(PLAYER_WALK);
		}
		if (move_count > -30)
		{
			move_count--;
		}
		if (!leftHit)
			rb->SetVelocityX((float)move_count);
		else
			rb->SetVelocityX(0.0f);

		PlayWalkSFX();
	}
	else if (move_count < 0)
	{
		if (input.KeyRelease(VK_A)) {
			move_count = 0;
			walk_count = 0;
		}
		else
			move_count++;
		if (move_count == 0 && !jumping && !inAir)
		{
			ChangeState(PLAYER_IDLE);
		}
		/*if (input.KeyRelease(VK_A) || (input.LeftAnalogStick().x > -0.1f && input.IsConnectController()))
		{
			move_count = 0;
			ChangeState(PLAYER_IDLE);
		}*/
	}


	if ((input.KeyTrigger(VK_W) || input.ButtonTrigger(XINPUT_A)) && isGround && !jumping /*&& !inWindow*/)
	{
		jump_count = 0;
		jumping = true;
		landCount = 0;
		landing = false;
		ChangeState(PLAYER_JUMP);
	}

	if ((input.KeyPress(VK_W) || input.ButtonPress(XINPUT_A)) && jumping /*&& !inWindow*/ && jump_count > 10)
	{
		if (inFloat)
			rb->AddForceImpulse({ 0,-15.0f + (float)jump_count / 2.0f });
		else
			rb->AddForceImpulse({ 0,15.0f - (float)jump_count / 2.0f });
	}

	// fall gravety
	/*float top = rb->GetVelocity().y;
	if (top <= 0.0f)
	{
		rb->AddForceImpulse({ 0,rb->GetGravityScale() * -0.25f });
	}*/

	if (jumping)
	{
		if (jump_count == 1) {
			Sound::Get().PlayWaveSound(SFX_Jump, 1.0f);
		}
		jump_count++;

		if (jump_count == 10)
		{
			if (inFloat)
				rb->AddForceImpulse({ 0,-50.0f });
			else
				rb->AddForceImpulse({ 0,50.0f });
		}
		if (jump_count > 30)
		{
			jump_count = 0;
			jumping = false;
			inAir = true;
			ChangeState(PLAYER_FLOAT);
		}
	}
	if (inAir && isGround)
	{
		inAir = false;
		Sound::Get().PlayWaveSound(SFX_Land, 1.0f);

		//エフェクト作成
		//=====================================================
		auto obj = LoadObject("asset/object/landingParticle.json");
		if (obj != nullptr)
		{
			obj->transform.position = m_this->transform.position;
			obj->transform.position.y += inFloat ? 50.0f : -50.0f;
			ParticleSystem* particle = nullptr;
			if (obj->TryGetComponent<ParticleSystem>(&particle)) {
				particle->Play();
			}
		}
		//=====================================================

		if (move_count != 0)
		{
			ChangeState(PLAYER_WALK);
		}
		else
		{
			landing = true;
			state->Land(mode, this);
		}
	}
	else if (landing)
	{
		landCount++;
		if (landCount > 20)
		{
			landCount = 0;
			landing = false;

			ChangeState(PLAYER_IDLE);
		}
	}
}
