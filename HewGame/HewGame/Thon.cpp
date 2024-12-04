#include "Thon.h"
#include "Scene_Gimmick.h"

void Thon::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2Dの使用(Def = 静的, type =動的)
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// コンポーネントを追加
	rb->CreateBoxShape(true);						// 当たり判定を作成
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