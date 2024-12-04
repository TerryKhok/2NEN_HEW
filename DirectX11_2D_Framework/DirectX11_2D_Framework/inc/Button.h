#pragma once

class Button : public Component
{
	friend class GameObject;

public:
	enum BUTTON_ACTION
	{
		MOUSE_TRIGGER,
		MOUSE_PRESS,
		MOUSE_RELEASE,
		ACTION_MAX
	};

	void SetEvent(std::function<void()>&& _func) {
		m_event = _func;
	}

	void SetAction(BUTTON_ACTION _action);
private:
	Button() {}
	//Button(std::function<void()> _func) {
	//	SetEvent(std::move(_func));
	//}

	void Update() { 
		(this->*pUpdate)(); 
	}

	void MouseTrigger();
	void MousePress();
	void MouseRelease();
private:
	void(Button::* pUpdate)() = &Button::MouseTrigger;
private:
	std::function<void(void)> m_event;
private:
	void DrawImGui() override
	{
		std::string str = m_event.target_type().name();
		const char* action = 
			pUpdate == &Button::MouseTrigger ? "mouseTrigger" :
			pUpdate == &Button::MouseTrigger ? "mousePress" :  "mouseRelease";
		ImGui::Text("action : %s", action);
		ImGui::Text("function :\n %s", str.substr(6).c_str());
	}
};