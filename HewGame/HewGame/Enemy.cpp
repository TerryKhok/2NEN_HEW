#include "Sample_Scene_Enemy.h"
#include "MovePlayer.h"
#include "Enemy.h"
#include "EnemyChaser.h"

void Enemy::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2Dの使用(Def = 静的, type =動的)
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// コンポーネントを追加
	rb->CreateBoxShape(true);						// 当たり判定を作成
}

void Enemy::Update()
{
	

}

void Enemy::OnColliderEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<Sample_Scene_Enemy>();
	}
}