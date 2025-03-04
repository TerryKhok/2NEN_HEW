#include "DecorativePlayerMove.h"

void DecorativePlayerMove::ChangeState(PLAYER_STATE _state)
{
	switch (_state)
	{
	case PLAYER_IDLE:
		state.reset(new DecorativePlayerIdle());
		break;
	case PLAYER_WALK:
		state.reset(new DecorativePlayerWalk());
		break;
	case PLAYER_JUMP:
		state.reset(new DecorativePlayerJump());
		break;
	case PLAYER_FLOAT:
		state.reset(new DecorativePlayerFloat());
		break;
	case PLAYER_DAMAGE:
		state.reset(new DecorativePlayerDamage());
		break;
	case PLAYER_GOAL:
		state.reset(new DecorativePlayerGoal());
		break;
	}

	state->Start(mode, this);
}

void DecorativePlayerMove::Update()
{
	auto& input = Input::Get();

	isGround = false;

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

	if (Box2D::WorldManager::RayCastShape(rayStart, rayEnd, rb, F_MAPRAY)) {
		isGround = true;
		airCount = 0;
	}
	else {
		if (airCount > 10)
		{
			if (!inAir)
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

		landing = true;
		state->Land(mode, this);
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
