#pragma once

class Thon : public Component
{
private:

	std::unordered_set<GameObject*> enters;
	SAFE_POINTER(Box2DBody, rb);

	void Start()
	{
		b2BodyDef bodyDef = b2DefaultBodyDef();			// BoxBody2D�̎g�p(Def = �ÓI, type =���I)
		rb = m_this->AddComponent<Box2DBody>(&bodyDef);	// �R���|�[�l���g��ǉ�
		rb->CreateBoxShape(true);						// �����蔻����쐬
	}

	void Update()
	{

	}

	/*void OnColliderEnter(GameObject* _ohter) 
	{ 
		if (enters.find(_other) != enters.end()) return;

	}*/
};