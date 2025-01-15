#include "YkTestScene.h"
#include "Enemy_type2.h"
#include "MovePlayer.h"

void Enemy_type2::Start()
{
	rb = m_this->GetComponent<Box2DBody>();
}

void Enemy_type2::Update()
{
	disCount++;

	// ���łɍő�ړ������܂Ői��ł�����܂�Ԃ��悤�ɂ���
	if (disCount == distance)
	{
		// 0 = ��, 1 = �E
		switch (direction)
		{
		case 0:
			direction = 1;
			disCount = 0;
			break;

		case 1:
			direction = 0;
			disCount = 0;
			break;

		}
	}

	// �ړ�����
	if (disCount < distance)
	{
		// 0 = ��, 1 = �E
		switch (direction)
		{
		case 0:
			m_this->transform.position.x -= speed;
			break;

		case 1:
			m_this->transform.position.x += speed;
			break;

		}
	}
}

void Enemy_type2::OnColliderEnter(GameObject* _other)
{
	MovePlayer* rb = nullptr;
	if (_other->TryGetComponent<MovePlayer>(&rb))
	{
		LOG(_other->GetName().c_str());
		SceneManager::LoadScene<YkTestScene>();
	}
}