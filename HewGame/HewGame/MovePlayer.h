#pragma once

#ifndef A_PLAYER
#define A_PLAYER

#include "MoveRect.h"

enum PLAYER_MODE
{
	NORMAL,
	ANTI_GRAVITY,
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

	void PlayWalkSFX() {
		walk_count++;
		if (walk_count == 1 && isGround) {
			std::random_device rnd;     // 非決定的な乱数生成器を生成
			std::mt19937 mt(rnd());     //  メルセンヌ・ツイスタの32ビット版、引数は初期シード値
			std::uniform_int_distribution<> rand4(1, 4);        // [1, 4] 範囲の一様乱数
			int RNG = rand4(mt);

			std::cerr << RNG << isGround << std::endl;
			switch (RNG) {
			case 1:
				Sound::Get().PlayWaveSound(SFX_Walk01, 1.0f);
				break;
			case 2:
				Sound::Get().PlayWaveSound(SFX_Walk02, 1.0f);
				break;
			case 3:
				Sound::Get().PlayWaveSound(SFX_Walk03, 1.0f);
				break;
			case 4:
				Sound::Get().PlayWaveSound(SFX_Walk04, 1.0f);
				break;
			}
		}
		if (walk_count == walk_speed) {
			walk_count = 0;
		}
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
		Sound::Get().PlayWaveSound(BGM_Game02, 0.3f);
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

	//goalヘッダーから呼び出してる
	void GameClear()
	{
		LOG("GameClear");
		Sound::Get().PlayWaveSound(SFX_Clear, 0.3f);
		ChangeState(PLAYER_GOAL);
		m_this->RemoveComponent<MovePlayer>();
	}

	void GameOver()
	{
		LOG("GameOver");
		Sound::Get().PlayWaveSound(SFX_Death, 0.5f);
		render->SetColor({ 1.0f,0.5f,0.5f,1.0f });
		ChangeState(PLAYER_DAMAGE);
		m_this->RemoveComponent<MovePlayer>();		
	}
private:
	bool isGround = false;
	bool jumping = false;
	int jump_count = 0;
	int move_count = 0; 
	int walk_count = 0;
	int walk_speed = 30;
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