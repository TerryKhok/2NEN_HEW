#pragma once

class MoveSubWindow : public Component
{
	friend class MoveSubWindowManager;

	SAFE_POINTER(Box2DBody, rb)
	SAFE_POINTER(SubWindow, subWindow)
	SAFE_POINTER(MoveSubWindowManager, subWndManager)

	void Proceed() override;
	void PauseUpdate();
	void Serialize(SERIALIZE_OUTPUT& ar) override;
	void Deserialize(SERIALIZE_INPUT& ar) override;

	const Vector2 GetStartPos() const
	{
		return m_this->transform.position;
	}

	void BackPosition();

	void Confirmed();
	
private:
	bool confirmed = false;
	int selfIndex = -1;
	float moveSpeed = 5.0f;

	//SERIALIZE_COMPONENT_VALUE(confirmed, selfIndex)
};

extern std::vector<std::stringstream> saveBuffer;

class Pause : public Component
{
	void Update()
	{
		DeleteObject(m_this);
		Window::PauseGame();
	}
};

class MoveSubWindowManager : public Component
{
	void Start() override
	{
		menu = false;
		/*auto pause = Instantiate("pause");
		pause->AddComponent<Pause>();*/
	}

	void Proceed() override
	{
		GameObject* handObject = ObjectManager::Find("handObject");
		if (handObject != nullptr)
		{
			handObject->SetActive(true);
		}
		GameObject* playObject = ObjectManager::Find("playObject");
		if (playObject != nullptr)
		{
			playObject->SetActive(false);
		}
		GameObject* moveObject = ObjectManager::Find("moveObject");
		if (moveObject != nullptr)
		{
			moveObject->SetActive(true);
		}

		/*std::sort(moveWindows.begin(), moveWindows.end(), [](const MoveSubWindow* _w1, const MoveSubWindow* _w2)
			{
				return _w1->selfIndex < _w2->selfIndex;
			});*/
	}

	void MoveSubWindowMode();
	void UndoGameSubWindow();
	void PlayGameSubWindow();

	void Update() override;
	void PauseUpdate() override;

	void ClearSaveBuffer()
	{
		saveBuffer.clear();
	}

	void DrawImGui(ImGuiApp::HandleUI& _handle) override
	{
		if (ImGui::Button("clear##moveSubwindowSaveBuffer"))
		{
			ClearSaveBuffer();
		}
	}

public:

	static std::vector<std::stringstream> saveBuffer;
	int selectIndex = 0;
	static bool menu;
	bool moveWindow = false;
	std::vector<MoveSubWindow*> moveWindows;

private:
	SERIALIZE_COMPONENT_VALUE(selectIndex)
};

SetReflectionComponent(MoveSubWindow)
SetReflectionComponent(MoveSubWindowManager)
