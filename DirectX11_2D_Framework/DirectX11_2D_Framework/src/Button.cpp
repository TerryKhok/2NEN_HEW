
void Button::SetAction(BUTTON_ACTION _action)
{
	switch (_action)
	{
	case MOUSE_TRIGGER:
		pUpdate = &Button::MouseTrigger;
		break;
	case MOUSE_PRESS:
		pUpdate = &Button::MousePress;
		break;
	case MOUSE_RELEASE:
		pUpdate = &Button::MouseRelease;
		break;
	default:
		break;
	}
}

void Button::MouseTrigger()
{
	if (Input::Get().MouseLeftTrigger())
	{
		const Vector2& mousePos = Input::Get().MousePoint();
		const Vector2& pos = m_this->transform.position;
		Vector2 scale = m_this->transform.scale;
		scale *= HALF_OBJECT_SIZE;
		if ((pos.x - scale.x) < mousePos.x &&
			(pos.x + scale.x) > mousePos.x &&
			(pos.y - scale.y) < mousePos.y &&
			(pos.y + scale.y) > mousePos.y)
		{
			m_event();
		}
	}
}

void Button::MousePress()
{
	if (Input::Get().MouseLeftPress())
	{
		const Vector2& mousePos = Input::Get().MousePoint();
		const Vector2& pos = m_this->transform.position;
		Vector2 scale = m_this->transform.scale;
		scale *= HALF_OBJECT_SIZE;
		if ((pos.x - scale.x) < mousePos.x &&
			(pos.x + scale.x) > mousePos.x &&
			(pos.y - scale.y) < mousePos.y &&
			(pos.y + scale.y) > mousePos.y)
		{
			m_event();
		}
	}
}

void Button::MouseRelease()
{
	if (Input::Get().MouseLeftRelease())
	{
		const Vector2& mousePos = Input::Get().MousePoint();
		const Vector2& pos = m_this->transform.position;
		Vector2 scale = m_this->transform.scale;
		scale *= HALF_OBJECT_SIZE;
		if ((pos.x - scale.x) < mousePos.x &&
			(pos.x + scale.x) > mousePos.x &&
			(pos.y - scale.y) < mousePos.y &&
			(pos.y + scale.y) > mousePos.y)
		{
			m_event();
		}
	}
}
