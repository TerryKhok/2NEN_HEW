#include "MoveSubWindow.h"

std::vector<std::stringstream> saveBuffer;

void MoveSubWindow::Proceed()
{
	rb = m_this->GetComponent<Box2DBody>();
	subWindow = m_this->GetComponent<SubWindow>();

	if (confirmed) return;

	auto manager = ObjectManager::Find("subWindowManager");
	if (manager != nullptr)
	{
		subWndManager = manager->GetComponent<MoveSubWindowManager>();
		selfIndex = (int)subWndManager->moveWindows.size();
		subWndManager->moveWindows.push_back(this);
	}
}

void MoveSubWindow::PauseUpdate()
{
	if (confirmed) return;

	if (selfIndex == subWndManager->selectIndex)
	{
		GameObject* handObject = ObjectManager::Find("handObject");
		if (handObject != nullptr)
		{
			Vector2 pos = m_this->transform.position;
			pos.x += m_this->transform.scale.x * HALF_OBJECT_SIZE;
			pos.x -= handObject->transform.scale.x * HALF_OBJECT_SIZE;
			pos.y -= m_this->transform.scale.y * HALF_OBJECT_SIZE;
			pos.y += handObject->transform.scale.y * HALF_OBJECT_SIZE;
			handObject->transform.position = pos;
		}

		auto& input = Input::Get();
		auto& pos = m_this->transform.position;

		if (input.KeyPress(VK_W) && pos.y < Window::MONITER_HALF_HEIGHT)
		{
			pos.y += moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_A) && pos.x > -Window::MONITER_HALF_WIDTH)
		{
			pos.x -= moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_S) && pos.y > -Window::MONITER_HALF_HEIGHT)
		{
			pos.y -= moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_D) && pos.x < Window::MONITER_HALF_WIDTH)
		{
			pos.x += moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
	}
}

void MoveSubWindow::SetPosition(Vector2 _pos)
{
	auto& pos = m_this->transform.position;
	pos = _pos;
	SetWindowPosition(subWindow->GeWndHandle(), pos);
	if (selfIndex == subWndManager->selectIndex)
	{
		GameObject* handObject = ObjectManager::Find("handObject");
		if (handObject != nullptr)
		{
			Vector2 pos = m_this->transform.position;
			pos.x += m_this->transform.scale.x * HALF_OBJECT_SIZE;
			pos.x -= handObject->transform.scale.x * HALF_OBJECT_SIZE;
			pos.y -= m_this->transform.scale.y * HALF_OBJECT_SIZE;
			pos.y += handObject->transform.scale.y * HALF_OBJECT_SIZE;
			handObject->transform.position = pos;
		}
	}
}

void MoveSubWindow::Confirmed()
{
	confirmed = true;
	auto check = Instantiate("checkIcon",L"asset/pic/checkmark.png");
	if (check != nullptr)
	{
		Vector2 pos = m_this->transform.position;
		pos.x += m_this->transform.scale.x * HALF_OBJECT_SIZE;
		pos.x -= check->transform.scale.x * HALF_OBJECT_SIZE;
		pos.y -= m_this->transform.scale.y * HALF_OBJECT_SIZE;
		pos.y += check->transform.scale.y * HALF_OBJECT_SIZE;
		check->transform.position = pos;
	}
	rb->SetPosition(m_this->transform.position);
}

void MoveSubWindowManager::MoveSubWindowMode()
{
	if (Window::IsPause()) return;

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

	Window::PauseGame();
}

void MoveSubWindowManager::UndoGameSubWindow()
{
	if (!saveBuffer.empty())
	{
		std::stringstream buffer = std::move(saveBuffer.back());
		saveBuffer.pop_back();
		SceneManager::LoadScene(buffer);
	}
}

void MoveSubWindowManager::PlayGameSubWindow()
{
	if (!Window::IsPause()) return;

	for (int i = (int)moveWindows.size() - 1;i >= 0;i--)
	{
		auto& window = moveWindows[i];
		window->SetPosition(startWindowPos.back());
		startWindowPos.pop_back();
	}

	GameObject* handObject = ObjectManager::Find("handObject");
	if (handObject != nullptr)
	{
		handObject->SetActive(false);
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
	Window::ResumeGame();
}


