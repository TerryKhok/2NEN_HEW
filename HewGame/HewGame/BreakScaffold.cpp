#include "BreakScaffold.h"
#include "MovePlayer.h"

void BreakScaffold::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2D�̎g�p(Def = �ÓI, type =���I)
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// �R���|�[�l���g��ǉ�
	rb->CreateBoxShape(true);						// �����蔻����쐬
}

void BreakScaffold::Update()
{
	if (touchFg == true)
	{
		DeleteObject(m_this);
	}
}

void BreakScaffold::OnColliderEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		touchFg = true;
	}
}