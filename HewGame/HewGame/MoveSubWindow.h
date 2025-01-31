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

	void BackPosition();

	void Confirmed();
	
private:
	bool confirmed = false;
	int selfIndex = -1;
	float moveSpeed = 5.0f;

	SERIALIZE_COMPONENT_VALUE(confirmed, selfIndex)
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
	}

	void MoveSubWindowMode();
	void UndoGameSubWindow();
	void PlayGameSubWindow();

	void Update() override
	{
		auto& input = Input::Get();

		if (input.KeyTrigger(VK_ESCAPE) || input.ButtonTrigger(XINPUT_GAMEPAD_START))
		{
			menu = !menu;
			if (menu)
			{
				moveWindow = false;
				LoadObject("asset/object/MenuText.json");
				LoadObject("asset/object/Interact.json");
				LoadObject("asset/object/SelectStage.json");
				LoadObject("asset/object/PlayAgain.json");
				Window::PauseGame();
			}
			else if (!moveWindow)
			{
				Window::ResumeGame();
			}
		}

		if (menu) return;

		if (input.KeyTrigger(VK_E) || input.ButtonTrigger(XINPUT_X))
		{
			if (saveBuffer.size() > 1)
			{
				saveBuffer.pop_back();
			}
			MoveSubWindowMode();
		}
		if (input.KeyTrigger(VK_R) || input.ButtonTrigger(XINPUT_B))
		{
		/*	if (saveBuffer.size() > 1)
			{
				saveBuffer.pop_back();
			}*/
			Window::PauseGame();
			UndoGameSubWindow();
		}
	}
	void PauseUpdate() override
	{
		auto& input = Input::Get();

		if (input.KeyTrigger(VK_ESCAPE) || input.ButtonTrigger(XINPUT_GAMEPAD_START))
		{
			menu = !menu;
			if (menu)
			{
				moveWindow = true;
				LoadObject("asset/object/MenuText.json");
				LoadObject("asset/object/Interact.json");
				LoadObject("asset/object/SelectStage.json");
				LoadObject("asset/object/PlayAgain.json");
			}
			else 
			{
				auto menuText = ObjectManager::Find("MenuText");
				if (menuText != nullptr)
				{
					DeleteObject(menuText);
				}
				auto Interact = ObjectManager::Find("Interact");
				if (Interact != nullptr)
				{
					DeleteObject(Interact);
				}
				auto SelectStage = ObjectManager::Find("SelectStage");
				if (SelectStage != nullptr)
				{
					DeleteObject(SelectStage);
				}
				auto PlayAgain = ObjectManager::Find("PlayAgain");
				if (PlayAgain != nullptr)
				{
					DeleteObject(PlayAgain);
				}
				if (!moveWindow)
					Window::ResumeGame();
			}
		}

		if (menu) return;

		if (input.KeyTrigger(VK_C) || input.ButtonTrigger(XINPUT_RIGHT_SHOULDER))
		{
			int size = (int)moveWindows.size();
			if (size > 0)
			{
				auto& index = selectIndex;
				index = (index + 1) % size;
			}
			Sound::Get().PlayWaveSound(SFX_Select, 0.3f);
		}
		if (input.KeyTrigger(VK_X) || input.ButtonTrigger(XINPUT_LEFT_SHOULDER))
		{
			int size = (int)moveWindows.size();
			if (size > 0)
			{
				auto& index = selectIndex;
				index = (index + size - 1) % size;
			}
			Sound::Get().PlayWaveSound(SFX_Select, 0.3f);
		}
		if (input.KeyTrigger(VK_E) || input.ButtonTrigger(XINPUT_X))
		{
			std::stringstream buffer;
			SceneManager::SaveScene(buffer);
			saveBuffer.push_back(std::move(buffer));

			PlayGameSubWindow();
		}
		if (input.KeyTrigger(VK_F) || input.ButtonTrigger(XINPUT_A))
		{
			if (selectIndex < moveWindows.size())
			{
				std::stringstream buffer;
				SceneManager::SaveScene(buffer);
				saveBuffer.push_back(std::move(buffer));

				moveWindows[selectIndex]->Confirmed();
				moveWindows.erase(moveWindows.begin() + selectIndex);
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
		if (input.KeyTrigger(VK_R) || input.ButtonTrigger(XINPUT_B))
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
