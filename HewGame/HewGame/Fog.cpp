#include "Fog.h"

void Fog::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2Dの使用(Def = 静的, type =動的)
	bodyDef.type = b2_dynamicBody;
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// コンポーネントを追加
}

void Fog::Update()
{
	
}