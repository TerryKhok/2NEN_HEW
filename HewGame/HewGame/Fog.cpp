#include "Fog.h"

void Fog::Start()
{
	b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2D�̎g�p(Def = �ÓI, type =���I)
	bodyDef.type = b2_dynamicBody;
	rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// �R���|�[�l���g��ǉ�
}

void Fog::Update()
{
	
}