#pragma once

class Thon : public Component
{
	std::string sceneName = "GameOverScene";
	int waitCount = 60;
	int count = 0;
	void(Thon::* pEvent)() = &Thon::VoidFunc;

	void VoidFunc() {}

	void EventFunc()
	{
		if (count == 0)
		{
			SceneManager::LoadingScene(sceneName);
		}

		count++;
		if (count > waitCount)
		{
			SceneManager::ChangeScene();
		}
	}

private:
	void Start();
	void Update();
	void OnColliderEnter(GameObject* _other) override;
	void OnCollisionEnter(GameObject* _other) override;
};