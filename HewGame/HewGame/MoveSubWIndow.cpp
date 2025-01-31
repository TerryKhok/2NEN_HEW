#include "MoveSubWindow.h"
#include "Permeation.h"

std::vector<std::stringstream> MoveSubWindowManager::saveBuffer;
bool MoveSubWindowManager::menu = false;

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
	if (confirmed || MoveSubWindowManager::menu) return;

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

		static Vector2 moveArea =
		{
			Window::MONITER_HALF_WIDTH * PROJECTION_ASPECT_WIDTH,
			Window::MONITER_HALF_HEIGHT * PROJECTION_ASPECT_HEIGHT
		};

		if (input.KeyPress(VK_W) && pos.y < moveArea.y)
		{
			pos.y += moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_A) && pos.x > -moveArea.x)
		{
			pos.x -= moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_S) && pos.y > -moveArea.y)
		{
			pos.y -= moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}
		if (input.KeyPress(VK_D) && pos.x < moveArea.x)
		{
			pos.x += moveSpeed;
			SetWindowPosition(subWindow->GeWndHandle(), pos);
		}

		if (input.IsConnectController())
		{
			Vector2 vec = input.LeftAnalogStick();
			if (vec.y > 0.1f && pos.y < moveArea.y)
			{
				pos.y += moveSpeed * vec.y;
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.x < -0.1f && pos.x > -moveArea.x)
			{
				pos.x += moveSpeed * vec.x;
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.y < -0.1f && pos.y > -moveArea.y)
			{
				pos.y += moveSpeed * vec.y;
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
			if (vec.x > 0.1f && pos.x < moveArea.x)
			{
				pos.x += moveSpeed * vec.x;
				SetWindowPosition(subWindow->GeWndHandle(), pos);
			}
		}
	}
}

void MoveSubWindow::BackPosition()
{
	auto& pos = m_this->transform.position;
	pos = rb->GetPosition();
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
	auto check = Instantiate("checkIcon", L"asset/pic/checkmark.png");
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

	std::vector<GameObject*> overlap;
	rb->GetOverlapObject(overlap);
	for (auto& obj : overlap)
	{
		Box2DBody* box2d = nullptr;
		if (obj->TryGetComponent<Box2DBody>(&box2d))
		{
			box2d->SetAwake(true);
		}
	}

	Permeation* permeation = nullptr;
	if (m_this->TryGetComponent <Permeation>(&permeation))
	{
		permeation->ClipObject();
	}
	Sound::Get().PlayWaveSound(SFX_Confirm, 0.3f);
}

void MoveSubWindowManager::MoveSubWindowMode()
{
	if (Window::IsPause()) return;

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
	Sound::Get().PlayWaveSound(SFX_Exit, .3f);
}

void MoveSubWindowManager::UndoGameSubWindow()
{
	if (!saveBuffer.empty())
	{
		Sound::Get().PlayWaveSound(SFX_Cancel, 0.3f);
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
		window->BackPosition();
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
	Sound::Get().PlayWaveSound(SFX_Open, 0.2f);
	Window::ResumeGame();
}


