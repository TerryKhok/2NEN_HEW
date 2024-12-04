#include "Thon.h"
#include "Scene_Gimmick.h"

void Thon::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2D�̎g�p(Def = �ÓI, type =���I)
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// �R���|�[�l���g��ǉ�
	rb->CreateBoxShape(true);						// �����蔻����쐬
}

void Thon::Update()
{

}

void Thon::OnColliderEnter(GameObject* _other)
{
	Box2DBody* rb = nullptr;
	if (_other->TryGetComponent<Box2DBody>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<SceneGimmick>();
	}
}