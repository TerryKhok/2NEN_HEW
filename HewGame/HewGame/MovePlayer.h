#pragma once

#ifndef A_PLAYER
#define A_PLAYER

#include "MoveRect.h"

class MovePlayer;

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

constexpr const wchar_t* wPlayerModeName[PLAYER_MODE_MAX] =
{
	L"normal",
	L"float",
	L"lowgravity",
	L"bounce",
	L"transparent",
};

struct PlayerState
{
	virtual void Start(PLAYER_MODE _mode, MovePlayer* _player) {}
	virtual void Update(PLAYER_MODE _mode, MovePlayer* _player) {}
	virtual void End(PLAYER_MODE _mode, MovePlayer* _player) {}
	virtual void Land(PLAYER_MODE _mode, MovePlayer* _player) {}
	virtual void ModeChange(PLAYER_MODE _mode, MovePlayer* _player) {}
	virtual void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) {}
protected:
	std::string clipName;
public:
	static constexpr const wchar_t* playerSpriteSheetRelativePath = L"asset/spritesheet/player_";
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
	SAFE_POINTER(ParticleSystem, particle)

	std::unique_ptr<PlayerState> state;

	void ChangeState(PLAYER_STATE _state);

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

		particle = m_this->GetComponent<ParticleSystem>();
	}
	PLAYER_MODE mode = NORMAL;
	std::vector<PLAYER_MODE> modeLayer;
public:
	bool inFloat = false;

	Animator* const GetAnimator() const{
		return anim;
	}

	ParticleSystem* const GetParticleSystem() const {
		return particle;
	}

	bool IsGround() const {
		return isGround;
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

	//goalヘッダーから呼び出してる
	void GameClear()
	{
		LOG("GameClear");
		
		Sound::Get().PlayWaveSound(SFX_Clear, 0.3f);
		ChangeState(PLAYER_GOAL);
		m_this->RemoveComponent<MovePlayer>();

		auto object = ObjectManager::Find("goalParticle");
		if (object == nullptr) return;
		object->SetActive(true);
		auto particle = object->GetComponent<ParticleSystem>();
		if (particle == nullptr) return;
		particle->Play();
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

	void PauseUpdate() override
	{
		SetModeTexture(NORMAL);
	}

	void Update();

	void Serialize(cereal::JSONOutputArchive& ar) override {
		if constexpr (1 <= 7) {
			::cereal::make_optional_nvp(ar, "reverse", reverse);
		}
		else {
			ar(reverse);
		}
	} void Deserialize(cereal::JSONInputArchive& ar) override {
		if constexpr (1 <= 7) {
			::cereal::make_optional_nvp(ar, "reverse", reverse);
		}
		else {
			ar(reverse);
		}
	}

public:
	static void SetPlayerNormal()
	{
		GameObject* obj = ObjectManager::Find("Player");
		if (obj != nullptr)
		{
			MovePlayer* player = nullptr;
			if (obj->TryGetComponent<MovePlayer>(&player))
			{
				player->SetModeTexture(PLAYER_MODE::NORMAL);
			}
		}
	}
};

SetReflectionComponent(MovePlayer)

struct PlayerIdle : public PlayerState
{
	void Start(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "StandBy";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, MovePlayer* _player) {
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
	void Land(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, MovePlayer* _player)
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

struct PlayerWalk : public PlayerState
{
	void Start(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Walk";
		_player->GetAnimator()->Play(clipName);

		_player->GetParticleSystem()->Resume();
	}
	void Update(PLAYER_MODE _mode, MovePlayer* _player)
	{
		auto particle = _player->GetParticleSystem();
		bool isGround = _player->IsGround();
		if (isGround)
			particle->Resume();
		else 
			particle->Pause();
	}
	void End(PLAYER_MODE _mode, MovePlayer* _player) {
		_player->GetParticleSystem()->Pause();
	}

	void Land(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, MovePlayer* _player)
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

struct PlayerJump : public PlayerState
{
	void Start(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Jump";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, MovePlayer* _player) {

	}
	void Land(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, MovePlayer* _player)
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

struct PlayerFloat : public PlayerState
{
	void Start(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Float";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, MovePlayer* _player) {

	}
	void Land(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Landing";
		_player->GetAnimator()->Play(clipName);
	}
	void ModeChange(PLAYER_MODE _mode, MovePlayer* _player)
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

struct PlayerDamage : public PlayerState
{
	void Start(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Damage";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, MovePlayer* _player) {

	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_damage.png";
		_rend->SetTexture(path.c_str());
	}
};

struct PlayerGoal : public PlayerState
{
	void Start(PLAYER_MODE _mode, MovePlayer* _player) {
		clipName = playerModeName[_mode];
		clipName += "Goal";
		_player->GetAnimator()->Play(clipName);
	}
	void Update(PLAYER_MODE _mode, MovePlayer* _player) {

	}
	void ChangeTexture(PLAYER_MODE _mode, Renderer* _rend) override
	{
		std::wstring path = playerSpriteSheetRelativePath;
		path += wPlayerModeName[_mode];
		path += L"_goal.png";
		_rend->SetTexture(path.c_str());
	}
};

#endif