#pragma once

#ifndef A_PLAYER
#define A_PLAYER

enum PLAYER_MODE
{
	NORMAL,
	UNTI_GRAVITY,
	LOW_GRAVITY,
	BOUNCE,
	PERMEATION,
	PLAYER_MODE_MAX
};

constexpr const char* playerModeName[PLAYER_MODE_MAX] =
{
	"normal",
	"float",
	"lowgravity",
	"bounce",
	"transparent",
};

struct PlayerState
{
	virtual void Start(PLAYER_MODE _mode, Animator* _ani) {}
	virtual void Update(PLAYER_MODE _mode, Animator* _ani) {}
	virtual void Land(PLAYER_MODE _mode, Animator* _ani) {}
	virtual void ModeChange(PLAYER_MODE _mode, Animator* _ani) {}
protected:
	std::string clipName;
};

struct PlayerIdle : public PlayerState
{
	void Start(PLAYER_MODE _mode, Animator* _ani){
		clipName = playerModeName[_mode];
		clipName += "StandBy";
		_ani->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, Animator* _ani){
		count++;
		if (count == 120)
		{
			int blink = rand() % 2;
			if (blink == 0)
			{
				clipName = playerModeName[_mode];
				clipName += "Blink";
				_ani->Play(clipName);
			}
		}
		if (count > 240)
		{
			count = 0;
			clipName = playerModeName[_mode];
			clipName += "StandBy";
			_ani->Play(clipName);
		}
	}
	void Land(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_ani->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, Animator* _ani)
	{
		clipName = playerModeName[_mode];
		clipName += "StandBy";
		_ani->Play(clipName);
	}
private:
	int count = 0;
};

struct PlayerWalk : public PlayerState
{
	void Start(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Walk";
		_ani->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, Animator* _ani)
	{
		clipName = playerModeName[_mode];
		clipName += "Walk";
		_ani->Play(clipName);
	}
};

struct PlayerJump : public PlayerState
{
	void Start(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Jump";
		_ani->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, Animator* _ani) {

	}
	void Land(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_ani->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, Animator* _ani)
	{
		clipName = playerModeName[_mode];
		clipName += "Float";
		_ani->Play(clipName);
	}
};

struct PlayerFloat : public PlayerState
{
	void Start(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Float";
		_ani->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, Animator* _ani) {

	}
	void Land(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_ani->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, Animator* _ani)
	{
		clipName = playerModeName[_mode];
		clipName += "Float";
		_ani->Play(clipName);
	}
};

struct PlayerDamage : public PlayerState
{
	void Start(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Damage";
		_ani->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, Animator* _ani) {

	}
};

struct PlayerGoal : public PlayerState
{
	void Start(PLAYER_MODE _mode, Animator* _ani) {
		clipName = playerModeName[_mode];
		clipName += "Goal";
		_ani->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, Animator* _ani) {

	}
};


class MovePlayer : public Component
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

	Sound sound;

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

		if (m_this->TryGetComponent<Animator>(&anim))
		{
			ChangeState(PLAYER_IDLE);
			anim->Reverse(reverse);
		}
		sound.PlayWaveSound(BGM_Game01, 1.0f);
	}
	PLAYER_MODE mode = NORMAL;
	std::vector<PLAYER_MODE> modeLayer;
public:
	bool inWindow = false;
	
	void SetMode(PLAYER_MODE _mode)
	{
		mode = _mode;
		modeLayer.push_back(mode);
		state->ModeChange(mode, anim);
	}
	void BackMode()
	{
		if (!modeLayer.empty())
			modeLayer.pop_back();

		if (modeLayer.empty()){
			mode = NORMAL;
		}
		else{
			mode = modeLayer.back();
		}
		state->ModeChange(mode, anim);
	}

	void GameOver()
	{
		LOG("GameOver");
	}
private:
	bool isGround = false;
	bool jumping = false;
	int jump_count = 0;
	int move_count = 0;
	int movement = 0;
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
		rayStart.y += 50.0f;
		Vector2 rayEnd = rayStart;
		rayEnd.y -= 60.0f;
		if (Box2D::WorldManager::RayCastShape(rayStart, rayEnd, rb, F_MAPRAY)){
			isGround = true;
			airCount = 0;
		}
		else{
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
		rayStart += {-50.0f, 5.0f};
		rayEnd = rayStart;
		rayEnd.x += 50.0f;
		if (Box2D::WorldManager::RayCastShape(rayStart, rayEnd,rb, F_MAPRAY)) {
			rightHit = true;
		}

		if ((input.KeyPress(VK_D) || input.LeftAnalogStick().x > 0.1f))
		{
			//if(isGround&&!sound.IsPlaying()){sound.PlayWaveSound(L"asset/sound/se/SFX_Walk01.wav", &waveData, false); }
			if (move_count == 1)
			{
				reverse = true;
				anim->Reverse(reverse);
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
				rb->SetVelocityX((float)move_count / rb->GetGravityScale());
		}
		else if (move_count > 0)
		{
			if (input.KeyRelease(VK_D))
				move_count = 0;
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
		rayStart += { 50.0f, 5.0f};
		rayEnd = rayStart;
		rayEnd.x -= 50.0f;
		if (Box2D::WorldManager::RayCastShape(rayStart, rayEnd, rb, F_MAPRAY)) {
			leftHit = true;
		}
		if ((input.KeyPress(VK_A) || input.LeftAnalogStick().x < -0.1f))
		{
			if (move_count == -1) 
			{
				reverse = false;
				anim->Reverse(reverse);
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
				rb->SetVelocityX((float)move_count / rb->GetGravityScale());
		}
		else if (move_count < 0)
		{
			if (input.KeyRelease(VK_A))
				move_count = 0;
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
		

		if ((input.KeyTrigger(VK_W) || input.ButtonTrigger(XINPUT_A)) && isGround && !jumping && !inWindow)
		{
			jump_count = 0;
			jumping = true;
			landCount = 0;
			landing = false;
			ChangeState(PLAYER_JUMP);
		}

		if ((input.KeyPress(VK_W) || input.ButtonPress(XINPUT_A)) && jumping && !inWindow && jump_count > 10)
		{
			rb->AddForceImpulse({ 0,15.0f - (float)jump_count / 2.0f });
		}

		// fall gravety
		float top = rb->GetVelocity().y;
		if (top <= 0.0f)
		{
			rb->AddForceImpulse({ 0,rb->GetGravityScale() * -0.25f });
		}

		if (jumping)
		{
			if (jump_count == 1) {
				sound.PlayWaveSound(SFX_Jump, 1.0f);
			}
			jump_count++;
			
			if (jump_count == 10)
			{
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
			sound.PlayWaveSound(SFX_Land, 1.0f);

			if (move_count != 0)
			{
				ChangeState(PLAYER_WALK);
			}
			else
			{
				landing = true;
				state->Land(mode, anim);
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

	SERIALIZE_COMPONENT_VALUE(reverse)
};

SetReflectionComponent(MovePlayer)

#endif