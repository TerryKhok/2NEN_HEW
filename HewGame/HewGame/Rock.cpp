#include "Scene_Gimmick.h"
#include "Rock.h"
#include "MovePlayer.h"


void Rock::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2D�̎g�p(Def = �ÓI, type =���I)
	bodyDef.type = b2_dynamicBody;
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// �R���|�[�l���g��ǉ�
	rb->CreateBoxShape(false);						// �����蔻����쐬
}

void Rock::Update()
{
	b2Vec2 vec = { rand() % 10 - 5,rand() % 10 - 5 };

	rb->AddForceImpule(vec);
}

void Rock::OnCollisionEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<SceneGimmick>();
	}
}