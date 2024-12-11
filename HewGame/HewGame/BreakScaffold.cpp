#include "BreakScaffold.h"
#include "MovePlayer.h"

void BreakScaffold::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2Dの使用(Def = 静的, type =動的)
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// コンポーネントを追加
	rb->CreateBoxShape(true);						// 当たり判定を作成
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