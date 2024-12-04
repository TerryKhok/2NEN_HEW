#pragma once

class Thon : public Component
{
private:

	std::unordered_set<GameObject*> enters;
	SAFE_POINTER(Box2DBody, rb);

	void Start()
	{
		b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2Dの使用(Def = 静的, type =動的)
		rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// コンポーネントを追加
		rb->CreateBoxShape(true);						// 当たり判定を作成
	}

	void Update()
	{

	}

	/*void OnColliderEnter(GameObject* _ohter) 
	{ 
		if (enters.find(_other) != enters.end()) return;

	}*/
};