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
		(this->*pUpdate)(); }

	void MouseTrigger();
	void MousePress();
	void MouseRelease();
private:
	void(Button::* pUpdate)() = &Button::MouseTrigger;
private:
	std::function<void(void)> m_event;
};