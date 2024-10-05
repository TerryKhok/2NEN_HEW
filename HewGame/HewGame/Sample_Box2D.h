#pragma once

class Box2D_SampleScene :public Scene
{
	void Load()
	{
		for (int y = -10; y < 65; y++)
		{
			for (int x = -20; x < 20; x++)
			{
				auto object = Instantiate("DynamicBox");
				object->transform.scale = { 1.0f,1.0f };
				object->transform.position = { static_cast<float>(x) * 2,static_cast<float>(y)};
				object->AddComponent<Renderer>(L"asset/pic/hartR.png");
				object->AddComponent<Box2DBody>()->CreateBoxShape();
			}
		}
	
		auto object = Instantiate("StaticBox");
		object->transform.position = { 0.0f,-200.0f };
		object->transform.scale = { 100.0f,2.0f };
		object->AddComponent<Renderer>(L"asset/pic/hartG.png");
		b2BodyDef bodyDef2 = b2DefaultBodyDef();
		bodyDef2.type = b2_kinematicBody;
		object->AddComponent<Box2DBody>(&bodyDef2)->CreateBoxShape();
	}

	Vector2 mousePos;
	Vector2 cameraPos;

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_G))
		{
			auto object = Instantiate("DynamicBox");
			object->transform.scale = { 1.0f,1.0f };
			object->transform.position.y = 100.0f;
			object->AddComponent<Renderer>(L"asset/pic/hartR.png");
			object->AddComponent<Box2DBody>()->CreateBoxShape();
		}

		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			SceneManager::LoadingScene<Box2D_SampleScene>();
			SceneManager::ChangeScene();
		}

		if (Input::Get().MouseRightTrigger())
		{
			mousePos = Input::Get().MousePoint();
			cameraPos = CameraManager::cameraPosition;
		}

		if (Input::Get().MouseRightPress())
		{
			Vector2 dis = Input::Get().MousePoint() - mousePos;
			dis *= -1.0f;
			CameraManager::cameraPosition = cameraPos + dis / 8;
		}

		if (Input::Get().MouseWheelDelta() > 0)
		{
			CameraManager::cameraZoom += 0.01f;
		}
		else if (Input::Get().MouseWheelDelta() < 0)
		{
			CameraManager::cameraZoom -= 0.01f;
		}
	}
};