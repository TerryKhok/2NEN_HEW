#pragma once

#include "MovePlayer.h"


class DecorativePlayerMove : public Component
{
		
	enum PLAYER_STATE
	{
		PLAYER_IDLE,
		PLAYER_WALK,
		PLAYER_JUMP,
		PLAYER_FLOAT,
		PLAYER_DAMAGE,
		PLAYER_GOAL,
		PLAYER_STATE_MAX
	};

	SAFE_POINTER(Renderer, render)
	SAFE_POINTER(Box2DBody, rb)
	SAFE_POINTER(Animator, anim)

	std::unique_ptr<PlayerState> state;

	void ChangeState(PLAYER_STATE _state)
	{
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

		state->Start(mode, anim);
	}

	void Start()
	{
		render = m_this->GetComponent<Renderer>();
		render->SetOffset({ 0.0f,15.0f * (float)cos(m_this->transform.angle.z.Get()) });

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

		if (m_this->TryGetComponent<Animator>(&anim))
		{
			ChangeState(PLAYER_IDLE);
			anim->Reverse(reverse);
		}
		Sound::Get().PlayWaveSound(BGM_Game01, 1.0f);
	}
	PLAYER_MODE mode = NORMAL;
	std::vector<PLAYER_MODE> modeLayer;
public:
	bool inFloat = false;

	void PushMode(PLAYER_MODE _mode)
	{
		mode = _mode;
		modeLayer.push_back(mode);
		state->ModeChange(mode, anim);
	}
	void PopMode(PLAYER_MODE _mode)
	{
		PLAYER_MODE oldMode = mode;
		if (!modeLayer.empty())
		{
			auto iter = std::find(modeLayer.begin(), modeLayer.end(), _mode);
			if (iter == modeLayer.end()) return;

			modeLayer.erase(iter);
		}

		if (modeLayer.empty()) {
			mode = NORMAL;
		}
		else {
			mode = modeLayer.back();
		}
		if (mode != oldMode)
			state->ModeChange(mode, anim);
	}


	void GameOver()
	{
		LOG("GameOver");
	}
private:
	bool isGround = false;
	bool inAir = false;
	int airCount = 0;
	bool landing = false;
	int landCount = 0;
	bool reverse = false;

	void Update()
	{
		auto& input = Input::Get();

		isGround = false;

		state->Update(mode, anim);

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

			landing = true;
			state->Land(mode, anim);
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
};

SetReflectionComponent(DecorativePlayerMove)
