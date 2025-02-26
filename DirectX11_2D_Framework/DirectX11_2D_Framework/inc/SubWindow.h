#pragma once

class SubWindow : public Component
{
	friend class GameObject;
	friend class ImGuiApp;

public:
	void DisableDrawLayer(LAYER _layer)
	{
		auto& viewMap = DirectX11::m_pRenderTargetViewList.first;
		auto& viewVec = DirectX11::m_pRenderTargetViewList.second;
		auto iter = viewMap.find(m_hWnd);
		if (iter != viewMap.end())
		{
			auto& layerList = viewVec[iter->second].layer;
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
		m_hWnd = Window::pWindowSubCreate(_object->GetName(),""/* _object->GetName()*/, width, height, pos);
	}

	SubWindow(GameObject* _object,const char* _windowName) {
		Vector2 size = _object->transform.scale;
		int width = static_cast<int>(size.x * DEFAULT_OBJECT_SIZE / PROJECTION_ASPECT_WIDTH);
		int height = static_cast<int>(size.y * DEFAULT_OBJECT_SIZE / PROJECTION_ASPECT_HEIGHT);
		Vector2 pos = _object->transform.position;
		m_hWnd = Window::pWindowSubCreate(_object->GetName(), _windowName, width, height, pos);
	}

	void Start() override
	{
		auto waveIter = DirectX11::m_waveHandleList.find(m_hWnd);
		if (waveIter != DirectX11::m_waveHandleList.end())
		{
			waveIter->second.first = isWave;
			if (!funcName.empty())
				waveIter->second.second = FunctionRegistry::Get().GetRegisterFunction(funcName);
		}
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

	void DrawImGui(ImGuiApp::HandleUI& _handleUi) override
	{
		ImGui::Checkbox("wave##SubWindow", &isWave);
		FunctionRegistry::DrawPickFunction("Event##SubWindow", funcName);
	}

private:
	HWND m_hWnd;
	bool isWave = false;
	std::string funcName;

	void Serialize(cereal::JSONOutputArchive& ar) override 
	{
		ar(CEREAL_NVP(isWave),CEREAL_NVP(funcName));
	} 
	
	void Deserialize(cereal::JSONInputArchive& ar) override
	{
		ar(CEREAL_NVP(isWave), CEREAL_NVP(funcName));
	}
};
