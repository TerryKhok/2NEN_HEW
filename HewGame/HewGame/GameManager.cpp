#include "GameManager.h"
#include "Trigger.h"
#include "MoveSubWindow.h"
#include "FixedSubWindowWorldPos.h"

namespace GameManager
{
	int currentStage = 0;

	void ChangeStage()
	{
		MoveSubWindowManager::saveBuffer.clear();

		std::string sceneName = stageName;
		if (currentStage < 10)
		{
			sceneName += "0";
		}
		sceneName += std::to_string(currentStage);

		RenderManager::renderOffset = { 0.0f,0.0f };

		SceneManager::LoadScene(sceneName);
	}

	void ChangeNextStage()
	{
		MoveSubWindowManager::saveBuffer.clear();

		std::string sceneName = GameManager::stageName;

		if (currentStage < 12)
		{
			currentStage++;
		}
		if (GameManager::currentStage < 10)
		{
			sceneName += "0";
		}
		sceneName += std::to_string(GameManager::currentStage);
		SceneManager::LoadScene(sceneName);
	}

	void ChangeCurrentStage()
	{
		MoveSubWindowManager::saveBuffer.clear();

		std::string sceneName = GameManager::stageName;

		if (GameManager::currentStage < 10)
		{
			sceneName += "0";
		}
		sceneName += std::to_string(GameManager::currentStage);
		SceneManager::LoadScene(sceneName);
	}

	static void ChangeTitleScene()
	{
		//SceneManager::LoadingScene("SelectScene");
		SceneManager::LoadScene("TitleScene");
	}

	REGISTER_FUNCTION(ChangeTitleScene)

	static void ChangeSelectScene()
	{
		auto ui = ObjectManager::Find("rollArrow");
		if (ui != nullptr)
		{
			ui->SetActive(true);
			auto rotate = ui->AddComponent<RotationObject>();
			rotate->rotationAngle = -Math::PI2;
			rotate->rotationTime = 1.0f;
		}

		SceneManager::LoadingScene("SelectScene");
		SceneManager::ChangeScene();
	}

	REGISTER_FUNCTION(ChangeNextStage)
	REGISTER_FUNCTION(ChangeCurrentStage)
	REGISTER_FUNCTION(ChangeSelectScene)

	void ChangeStage1()
	{
		GameManager::currentStage = 1;
		GameManager::ChangeStage();
	}

	void ChangeStage2()
	{
		GameManager::currentStage = 2;
		GameManager::ChangeStage();
	}

	void ChangeStage3()
	{
		GameManager::currentStage = 3;
		GameManager::ChangeStage();
	}

	void ChangeStage4()
	{
		GameManager::currentStage = 4;
		GameManager::ChangeStage();
	}

	void ChangeStage5()
	{
		GameManager::currentStage = 5;
		GameManager::ChangeStage();
	}

	void ChangeStage6()
	{
		GameManager::currentStage = 6;
		GameManager::ChangeStage();
	}

	void ChangeStage7()
	{
		GameManager::currentStage = 7;
		GameManager::ChangeStage();
	}

	void ChangeStage8()
	{
		GameManager::currentStage = 8;
		GameManager::ChangeStage();
	}

	void ChangeStage9()
	{
		GameManager::currentStage = 9;
		GameManager::ChangeStage();
	}

	void ChangeStage10()
	{
		GameManager::currentStage = 10;
		GameManager::ChangeStage();
	}

	void ChangeStage11()
	{
		GameManager::currentStage = 11;
		GameManager::ChangeStage();
	}

	void ChangeStage12()
	{
		GameManager::currentStage = 12;
		GameManager::ChangeStage();
	}

	REGISTER_FUNCTION(ChangeStage1)
	REGISTER_FUNCTION(ChangeStage2)
	REGISTER_FUNCTION(ChangeStage3)
	REGISTER_FUNCTION(ChangeStage4)
	REGISTER_FUNCTION(ChangeStage5)
	REGISTER_FUNCTION(ChangeStage6)
	REGISTER_FUNCTION(ChangeStage7)
	REGISTER_FUNCTION(ChangeStage8)
	REGISTER_FUNCTION(ChangeStage9)
	REGISTER_FUNCTION(ChangeStage10)
	REGISTER_FUNCTION(ChangeStage11)
	REGISTER_FUNCTION(ChangeStage12)

	void SetFixedSubWindow(GameObject* obj)
	{
		if (obj == nullptr) return;

		bool active = obj->IsActive();
		if (!active)
		{
			obj->AddComponent<SubWindow>();
			obj->AddComponent<FixedSubWindowWorldPos>();
		}
		else
		{
			obj->RemoveComponent<SubWindow>();
			obj->RemoveComponent<FixedSubWindowWorldPos>();
		}

		obj->SetActive(!active);
	}

	static void MoveCameraLeft()
	{
		auto ui = ObjectManager::Find("LeftStage");
		if (ui != nullptr)
		{
			auto rotation = ui->AddComponent<RotationObject>();
		}
		SetFixedSubWindow(ObjectManager::Find("RightStage"));
		SetFixedSubWindow(ObjectManager::Find("BottomStage"));
		for (int i = 1; i <= 4; i++)
		{
			std::string name = "stage" + std::to_string(i);
			SetFixedSubWindow(ObjectManager::Find(name));
		}

		auto chaseCamera = ObjectManager::Find("chaseCameraLeft");
		if (chaseCamera != nullptr)
		{
			chaseCamera->SetActive(true);
		}
	}

	REGISTER_FUNCTION(MoveCameraLeft)

	static void MoveCameraRight()
	{
		auto ui = ObjectManager::Find("RightStage");
		if (ui != nullptr)
		{
			auto rotation = ui->AddComponent<RotationObject>();
		}
		SetFixedSubWindow(ObjectManager::Find("LeftStage"));
		SetFixedSubWindow(ObjectManager::Find("BottomStage"));
		for (int i = 5; i <= 8; i++)
		{
			std::string name = "stage" + std::to_string(i);
			SetFixedSubWindow(ObjectManager::Find(name));
		}

		auto chaseCamera = ObjectManager::Find("chaseCameraRight");
		if (chaseCamera != nullptr)
		{
			chaseCamera->SetActive(true);
		}
	}

	REGISTER_FUNCTION(MoveCameraRight)

	static void MoveCameraBottom()
	{
		auto ui = ObjectManager::Find("BottomStage");
		if (ui != nullptr)
		{
			auto rotation = ui->AddComponent<RotationObject>();
			auto moveVer = ui->AddComponent<MoveVerticalPoint>();
			moveVer->endPos = ui->transform.position;
			moveVer->endPos.x += (float)cos(ui->transform.angle.z.Get()) * 200.0f;
			moveVer->endPos.y -= (float)sin(ui->transform.angle.z.Get()) * 200.0f;
		}
		SetFixedSubWindow(ObjectManager::Find("LeftStage"));
		SetFixedSubWindow(ObjectManager::Find("RightStage"));
		for (int i = 9; i <= 12; i++)
		{
			std::string name = "stage" + std::to_string(i);
			SetFixedSubWindow(ObjectManager::Find(name));
		}

		auto chaseCamera = ObjectManager::Find("chaseCameraBottom");
		if (chaseCamera != nullptr)
		{
			chaseCamera->SetActive(true);
		}
	}

	REGISTER_FUNCTION(MoveCameraBottom)
}


