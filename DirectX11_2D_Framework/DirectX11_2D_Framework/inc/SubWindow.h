#pragma once

class SubWindow : public Component
{
	friend class GameObject;
	friend class ImGuiApp;

public:
	void DisableDrawLayer(LAYER _layer)
	{
		auto& viewList = DirectX11::m_pRenderTargetViewList;
		auto iter = viewList.find(m_hWnd);
		if (iter != viewList.end())
		{
			auto& layerList = iter->second.second;
			auto it = std::find(layerList.begin(), layerList.end(), _layer);
			if (it != layerList.end())
			{
				layerList.erase(it);
			}
		}
	}

	const HWND GeWndHandle() const
	{
		return m_hWnd;
	}
private:
	SubWindow(GameObject* _object){
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE / PROJECTION_ASPECT_WIDTH);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE / PROJECTION_ASPECT_HEIGHT);
		Vector2 pos = _object->transform.position;
		pos.x /= PROJECTION_ASPECT_WIDTH;
		pos.y /= PROJECTION_ASPECT_HEIGHT;
		m_hWnd = Window::pWindowSubCreate(_object->GetName(), _object->GetName(), width, height, pos);
	}

	SubWindow(GameObject* _object,const char* _windowName) {
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE / PROJECTION_ASPECT_WIDTH);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE / PROJECTION_ASPECT_HEIGHT);
		Vector2 pos = _object->transform.position;
		pos.x /= PROJECTION_ASPECT_WIDTH;
		pos.y /= PROJECTION_ASPECT_HEIGHT;
		m_hWnd = Window::pWindowSubCreate(_object->GetName(), _windowName, width, height, pos);
	}

	void Delete() override
	{
		Window::WindowSubRelease(m_hWnd);
	}

	void SetActive(bool _active) override
	{
		_active ? ShowWindow(m_hWnd, SW_SHOW) : ShowWindow(m_hWnd, SW_HIDE);
	}

	void OnWindowMove(HWND _hWnd, RECT* _rect) override
	{
		m_this->transform.position = GetWindowPosition(_hWnd);
	}

private:
	HWND m_hWnd;
};
