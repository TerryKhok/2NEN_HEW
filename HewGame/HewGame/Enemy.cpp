#include "Sample_Scene_Enemy.h"
#include "MovePlayer.h"
#include "Enemy.h"
#include "EnemyChaser.h"

void Enemy::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2D�̎g�p(Def = �ÓI, type =���I)
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// �R���|�[�l���g��ǉ�
	rb->CreateBoxShape(true);						// �����蔻����쐬
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