#pragma once

#include "Trigger.h"

class MoveRect : public Component
{
	SAFE_POINTER(Box2DBody, rb)
	SAFE_POINTER(SubWindow, subWindow)
	float moveSpeed = 5.0f;

	void Start() override
	{
		rb = m_this->GetComponent<Box2DBody>();
		subWindow = m_this->GetComponent<SubWindow>();
	}

	void Update()
	{
		auto& input = Input::Get();
		auto& pos = m_this->transform.position;
		Vector2 windowPos = GetWindowPosition(subWindow->GeWndHandle());
		if (windowPos != m_this->transform.position)
		{
			rb->SetPosition(windowPos);
		}

		static Vector2 moveArea =
		{
			Window::MONITER_HALF_WIDTH * PROJECTION_ASPECT_WIDTH,
			Window::MONITER_HALF_HEIGHT * PROJECTION_ASPECT_HEIGHT
		};

		auto& offset = RenderManager::renderOffset;

		bool move = false;
		if (input.KeyPress(VK_W) && pos.y < moveArea.y + offset.y)
		{
			move = true;
			pos.y += moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_A) && pos.x > -moveArea.x + offset.x)
		{
			move = true;
			pos.x -= moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_S) && pos.y > -moveArea.y + offset.y)
		{
			move = true;
			pos.y -= moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_D) && pos.x < moveArea.x + offset.x)
		{
			move = true;
			pos.x += moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}

		if (input.IsConnectController())
		{
			Vector2 vec = input.LeftAnalogStick();
			if (vec.y > 0.1f && pos.y < moveArea.y + offset.y)
			{
				move = true;
				pos.y += moveSpeed * vec.y;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.x < -0.1f && pos.x > -moveArea.x + offset.x)
			{
				move = true;
				pos.x += moveSpeed * vec.x;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.y < -0.1f && pos.y > -moveArea.y + offset.y)
			{
				move = true;
				pos.y += moveSpeed * vec.y;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.x > 0.1f && pos.x < moveArea.x + offset.x)
			{
				move = true;
				pos.x += moveSpeed * vec.x;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
		}

		if (move)
		{
			std::vector<GameObject*> overlap;
			rb->GetOverlapObject(overlap);
			for (auto& obj : overlap)
			{
				auto box2d = obj->GetComponent<Box2DBody>();
				box2d->SetAwake(true);
			}
		}
	}

	void OnColliderEnter(GameObject* _other) override
	{
		Box2DBody* box2d = nullptr;
		if (_other->TryGetComponent<Box2DBody>(&box2d))
		{
			box2d->SetAwake(true);
		}
	}
};

SetReflectionComponent(MoveRect)

class MoveRectPause : public Component
{
	SAFE_POINTER(Box2DBody, rb)
	SAFE_POINTER(SubWindow, subWindow)
	float moveSpeed = 5.0f;

	void Start() override
	{
		rb = m_this->GetComponent<Box2DBody>();
		subWindow = m_this->GetComponent<SubWindow>();
	}

	void PauseUpdate()
	{
		auto& input = Input::Get();
		auto& pos = m_this->transform.position;
		Vector2 windowPos = GetWindowPosition(subWindow->GeWndHandle());
		if (windowPos != m_this->transform.position)
		{
			rb->SetPosition(windowPos);
		}

		static Vector2 moveArea =
		{
			Window::MONITER_HALF_WIDTH * PROJECTION_ASPECT_WIDTH,
			Window::MONITER_HALF_HEIGHT * PROJECTION_ASPECT_HEIGHT
		};

		auto& offset = RenderManager::renderOffset;

		bool move = false;
		if (input.KeyPress(VK_W) && pos.y < moveArea.y + offset.y)
		{
			move = true;
			pos.y += moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_A) && pos.x > -moveArea.x + offset.x)
		{
			move = true;
			pos.x -= moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_S) && pos.y > -moveArea.y + offset.y)
		{
			move = true;
			pos.y -= moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_D) && pos.x < moveArea.x + offset.x)
		{
			move = true;
			pos.x += moveSpeed;
			rb->SetPosition(pos);
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}

		if (input.IsConnectController())
		{
			Vector2 vec = input.LeftAnalogStick();
			if (vec.y > 0.1f && pos.y < moveArea.y + offset.y)
			{
				move = true;
				pos.y += moveSpeed * vec.y;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.x < -0.1f && pos.x > -moveArea.x + offset.x)
			{
				move = true;
				pos.x += moveSpeed * vec.x;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.y < -0.1f && pos.y > -moveArea.y + offset.y)
			{
				move = true;
				pos.y += moveSpeed * vec.y;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.x > 0.1f && pos.x < moveArea.x + offset.x)
			{
				move = true;
				pos.x += moveSpeed * vec.x;
				rb->SetPosition(pos);
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
		}

		if (move)
		{
			std::vector<GameObject*> overlap;
			rb->GetOverlapObject(overlap);
			for (auto& obj : overlap)
			{
				auto box2d = obj->GetComponent<Box2DBody>();
				box2d->SetAwake(true);
			}
		}
	}

	void OnColliderEnter(GameObject* _other) override
	{
		Box2DBody* box2d = nullptr;
		if (_other->TryGetComponent<Box2DBody>(&box2d))
		{
			box2d->SetAwake(true);
		}
	}
};

SetReflectionComponent(MoveRectPause)