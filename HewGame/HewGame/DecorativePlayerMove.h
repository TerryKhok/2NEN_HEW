#pragma once

#include "MovePlayer.h"

class DecorativePlayerMove;

struct DecorativePlayerState
{
	virtual void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {}
	virtual void Update(PLAYER_MODE _mode, DecorativePlayerMove* _player) {}
	virtual void Land(PLAYER_MODE _mode, DecorativePlayerMove* _player) {}
	virtual void ModeChange(PLAYER_MODE _mode, DecorativePlayerMove* _player) {}
	virtual void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) {}
protected:
	std::string clipName;
public:
	static constexpr const wchar_t* playerSpriteSheetRelativePath = L"asset/spritesheet/player_";
};

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

	std::unique_ptr<DecorativePlayerState> state;

	void ChangeState(PLAYER_STATE _state);

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

	Animator* const GetAnimator() const {
		return anim;
	}

	void SetModeTexture(PLAYER_MODE _mode)
	{
		if (landing)
		{
			std::wstring path = PlayerState::playerSpriteSheetRelativePath;
			path += wPlayerModeName[_mode];
			path += L"_landing.png";
			render->SetTexture(path.c_str());
		}
		else
			state->ChangeTexture(_mode, render);
	}

	void PushMode(PLAYER_MODE _mode)
	{
		mode = _mode;
		modeLayer.push_back(mode);
		state->ModeChange(mode, this);
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
			state->ModeChange(mode, this);
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

	void PauseUpdate() override
	{
		SetModeTexture(NORMAL);
	}

	void Update();

	public:
		static void SetDecorativePlayerNormal()
		{
			GameObject* obj = ObjectManager::Find("Player");
			if (obj != nullptr)
			{
				DecorativePlayerMove* player = nullptr;
				if (obj->TryGetComponent<DecorativePlayerMove>(&player))
				{
					player->SetModeTexture(PLAYER_MODE::NORMAL);
				}
			}
		}
};

SetReflectionComponent(DecorativePlayerMove)

struct DecorativePlayerIdle : public DecorativePlayerState
{
	void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "StandBy";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		count++;
		if (count == 120)
		{
			int blink = rand() % 2;
			if (blink == 0)
			{
				clipName = playerModeName[_mode];
				clipName += "Blink";
				_player->GetAnimator()->Play(clipName);
			}
		}
		if (count > 240)
		{
			count = 0;
			clipName = playerModeName[_mode];
			clipName += "StandBy";
			_player->GetAnimator()->Play(clipName);
		}
	}
	void Land(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, DecorativePlayerMove* _player)
	{
		clipName = playerModeName[_mode];
		clipName += "StandBy";
		_player->GetAnimator()->Play(clipName);
	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_standby.png";
		_rend->SetTexture(path.c_str());
	}
private:
	int count = 0;
};

struct DecorativePlayerWalk : public DecorativePlayerState
{
	void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Walk";
		_player->GetAnimator()->Play(clipName);
	}
	void Land(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, DecorativePlayerMove* _player)
	{
		clipName = playerModeName[_mode];
		clipName += "Walk";
		_player->GetAnimator()->Play(clipName);
	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_walk.png";
		_rend->SetTexture(path.c_str());
	}
};

struct DecorativePlayerJump : public DecorativePlayerState
{
	void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Jump";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, DecorativePlayerMove* _player) {

	}
	void Land(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, DecorativePlayerMove* _player)
	{
		clipName = playerModeName[_mode];
		clipName += "Float";
		_player->GetAnimator()->Play(clipName);
	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_jump.png";
		_rend->SetTexture(path.c_str());
	}
};

struct DecorativePlayerFloat : public DecorativePlayerState
{
	void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Float";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, DecorativePlayerMove* _player) {

	}
	void Land(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, DecorativePlayerMove* _player)
	{
		clipName = playerModeName[_mode];
		clipName += "Float";
		_player->GetAnimator()->Play(clipName);
	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_float.png";
		_rend->SetTexture(path.c_str());
	}
};

struct DecorativePlayerDamage : public DecorativePlayerState
{
	void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Damage";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, DecorativePlayerMove* _player) {

	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_damage.png";
		_rend->SetTexture(path.c_str());
	}
};

struct DecorativePlayerGoal : public DecorativePlayerState
{
	void Start(PLAYER_MODE _mode, DecorativePlayerMove* _player) {
		clipName = playerModeName[_mode];
		clipName += "Goal";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, DecorativePlayerMove* _player) {

	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_goal.png";
		_rend->SetTexture(path.c_str());
	}
};
