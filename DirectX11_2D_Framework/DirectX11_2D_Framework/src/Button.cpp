
void Button::SetEvent(std::string _funcName)
{
	auto iter = FunctionRegistry::functions.find(_funcName);
	if (iter != FunctionRegistry::functions.end())
	{
		m_funcName = iter->first;
		m_event = iter->second;
	}
}

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

	m_action = _action;
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

void Button::Serialize(SERIALIZE_OUTPUT& ar)
{
	std::string funcName(m_funcName);
	BUTTON_ACTION& action = m_action;
	ar(CEREAL_NVP(funcName), CEREAL_NVP(action));
}

void Button::Deserialize(SERIALIZE_INPUT& ar)
{
	std::string funcName;
	BUTTON_ACTION action;
	ar(CEREAL_NVP(funcName), CEREAL_NVP(action));
	SetEvent(funcName);
	SetAction(action);
}

void Button::DrawImGui(ImGuiApp::HandleUI& _handle)
{
	if (ImGui::BeginCombo("action##button", magic_enum::enum_name(m_action).data()))
	{
		for (int i = 0; i < ACTION_MAX; i++)
		{
			BUTTON_ACTION action = (BUTTON_ACTION)i;
			bool same = m_action == action;
			if (ImGui::Selectable(magic_enum::enum_name(action).data(), &same))
			{
				SetAction(action);
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("event##button", m_funcName.data()))
	{
		for (auto& func : FunctionRegistry::functions)
		{
			bool same = m_funcName == func.first;
			if (ImGui::Selectable(func.first.c_str(),&same))
			{
				SetEvent(func.first);
			}
		}
		ImGui::EndCombo();
	}
}

void ButtonLog()
{
	LOG("Execute");
}

void FunctionRegistry::DrawPickFunction(const char* _label, std::string& _funcName)
{
	if (ImGui::BeginCombo(_label, _funcName.data()))
	{
		for (auto& func : FunctionRegistry::functions)
		{
			bool same = _funcName == func.first;
			if (ImGui::Selectable(func.first.c_str(), &same))
			{
				_funcName = func.first;
			}
		}
		ImGui::EndCombo();
	}
}
