#pragma once

class Rock : public Component
{
	SAFE_POINTER(Box2DBody, rb)

	std::string sceneName = "GameOverScene";
	int waitCount = 60;
	int count = 0;
	void(Rock::* pEvent)() = &Rock::VoidFunc;

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