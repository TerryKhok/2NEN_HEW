#pragma once

class MovePlayer : public Component
{
	SAFE_POINTER(Box2DBody, rb)
	
	void Start()
	{
		rb = m_this->GetComponent<Box2DBody>();
		if (rb == nullptr)
		{
			m_this->AddComponent<Box2DBody>();
		}
	}

	void Update()
	{
		if (Input::Get().KeyPress(VK_D))
		{
			rb->SetVelocityX(10.0f);
		}
		if (Input::Get().KeyPress(VK_A))
		{
			rb->SetVelocityX(-10.0f);
		}
		if (Input::Get().KeyPress(VK_W))
		{
			rb->SetVelocityY(10.0f);
		}
		if (Input::Get().KeyPress(VK_S))
		{
			rb->SetVelocityY(-10.0f);
		}
	}
};
