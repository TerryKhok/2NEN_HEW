#pragma once

class MoveSubWindow : public Component
{
	friend class MoveSubWindowManager;

	SAFE_POINTER(Box2DBody, rb)
	SAFE_POINTER(SubWindow, subWindow)
	SAFE_POINTER(MoveSubWindowManager, subWndManager)

	void Proceed() override;

	void PauseUpdate();

	const Vector2 GetStartPos() const
	{
		return m_this->transform.position;
	}

	void SetPosition(Vector2 _pos);

	void Confirmed();
	
private:
	bool confirmed = false;
	int selfIndex = -1;
	float moveSpeed = 5.0f;

	SERIALIZE_COMPONENT_VALUE(confirmed, selfIndex)
};

extern std::vector<std::stringstream> saveBuffer;

class MoveSubWindowManager : public Component
{
	void Start() override
	{
		Window::PauseGame();
	}

	void Proceed() override
	{
		for (auto& window : moveWindows)
		{
			startWindowPos.push_back(window->GetStartPos());
		}

		GameObject* handObject = ObjectManager::Find("handObject");
		if (handObject != nullptr)
		{
			handObject->SetActive(true);
		}
		GameObject* playObject = ObjectManager::Find("playObject");
		if (playObject != nullptr)
		{
			playObject->SetActive(true);
		}
		GameObject* moveObject = ObjectManager::Find("moveObject");
		if (moveObject != nullptr)
		{
			moveObject->SetActive(false);
		}
	}

	void MoveSubWindowMode();
	void UndoGameSubWindow();
	void PlayGameSubWindow();

	void Update() override
	{
		if (Input::Get().KeyTrigger(VK_E))
		{
			MoveSubWindowMode();
		}
		if (Input::Get().KeyTrigger(VK_R))
		{
			UndoGameSubWindow();
		}
	}
	void PauseUpdate() override
	{
		if (Input::Get().KeyTrigger(VK_C))
		{
			if (moveWindows.size() > 0)
			{
				auto& index = selectIndex;
				index = (index + 1) % moveWindows.size();
			}
		}
		if (Input::Get().KeyTrigger(VK_E))
		{
			PlayGameSubWindow();
		}
		if (Input::Get().KeyTrigger(VK_F))
		{
			if (selectIndex < moveWindows.size())
			{
				std::stringstream buffer;
				SceneManager::SaveScene(buffer);
				saveBuffer.push_back(std::move(buffer));

				moveWindows[selectIndex]->Confirmed();
				moveWindows.erase(moveWindows.begin() + selectIndex);
				startWindowPos.erase(startWindowPos.begin() + selectIndex);
				for (int i = 0;i < (int)moveWindows.size();i++)
				{
					moveWindows[i]->selfIndex = i;
				}
				selectIndex = 0;
				if (moveWindows.empty())
				{
					GameObject* handObject = ObjectManager::Find("handObject");
					if (handObject != nullptr)
					{
						DeleteObject(handObject);
					}
				}
			}
		}
		if (Input::Get().KeyTrigger(VK_R))
		{
			UndoGameSubWindow();
		}
	}

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
	int selectIndex = 0;
	std::vector<MoveSubWindow*> moveWindows;
	std::vector<Vector2> startWindowPos;

private:
	SERIALIZE_COMPONENT_VALUE(selectIndex)
};

SetReflectionComponent(MoveSubWindow)
SetReflectionComponent(MoveSubWindowManager)
