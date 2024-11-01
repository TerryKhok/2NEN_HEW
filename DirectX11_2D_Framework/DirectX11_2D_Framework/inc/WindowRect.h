#pragma once

class WindowRect : public Component
{
public:
	WindowRect(GameObject* _object){
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE);
		m_hWnd = Window::pWindowSubCreate(_object->GetName(),"SUB", width, height);

		Vector2 pos = GetWindowPosition(m_hWnd);
		_object->transform.position = pos;

		auto rb = _object->GetComponent<Box2DBody>();
		if (rb != nullptr)
		{
			rb->SetPosition(pos);
		}
	}

	WindowRect(GameObject* _object,const char* _windowName) {
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE);
		m_hWnd = Window::pWindowSubCreate(_object->GetName(), _windowName, width, height);

		SetWindowPosition(m_hWnd, _object->transform.position);

		/*Vector2 pos = GetWindowPosition(m_hWnd);
		_object->transform.position = pos;*/

		auto rb = _object->GetComponent<Box2DBody>();
		if (rb != nullptr)
		{
			rb->SetPosition(_object->transform.position);
		}
	}

	void Delete() override
	{
		Window::WindowSubRelease(m_hWnd);
	}

	void OnWindowMove(HWND _hWnd) override
	{
		m_this->transform.position = GetWindowPosition(_hWnd);
	}

private:
	HWND m_hWnd;
};
