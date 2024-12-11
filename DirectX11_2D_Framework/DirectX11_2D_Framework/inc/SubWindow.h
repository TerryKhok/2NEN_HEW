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
private:
	SubWindow(GameObject* _object){
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE);
		m_hWnd = Window::pWindowSubCreate(_object->GetName(), _object->GetName(), width, height, _object->transform.position);
	}

	SubWindow(GameObject* _object,const char* _windowName) {
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE);
		m_hWnd = Window::pWindowSubCreate(_object->GetName(), _windowName, width, height, _object->transform.position);
	}

	void Delete() override
	{
		Window::WindowSubRelease(m_hWnd);
	}

	void SetActive(bool _active) override
	{
		if (_active)
		{
			ShowWindow(m_hWnd, SW_SHOW);
		}
		else
		{
			ShowWindow(m_hWnd, SW_HIDE);
		}
	}

	void OnWindowMove(HWND _hWnd, RECT* _rect) override
	{
		m_this->transform.position = GetWindowPosition(_hWnd);
	}

private:
	HWND m_hWnd;
};
