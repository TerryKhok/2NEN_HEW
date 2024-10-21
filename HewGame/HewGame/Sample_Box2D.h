#pragma once

class SampleScene_Box2D :public Scene
{
	SAFE_POINTER(Box2DBody, box2dbody)
	SAFE_POINTER(GameObject, gameObject)

	void Load()
	{
		std::vector<b2Vec2> mesh =
		{
			{-25,25},{-30,10} ,{-25,-25},{ 0,-50 },{25,-25},{30,10} ,{25,25},{0,50}
		};

		gameObject = Instantiate("DynamicBox");
		gameObject->AddComponent<Renderer>(L"asset/pic/hartR.png");

		/*auto text = gameObject->AddComponent<DWText>();
		text->SetText("aaaa");*/

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.fixedRotation = true;
		bodyDef.type = b2_dynamicBody;
		//bodyDef.gravityScale = 0.0f;
		box2dbody = gameObject->AddComponent<Box2DBody>(&bodyDef);
		//box2dbody->CreateCapsuleShape();
		box2dbody->CreatePolygonShape(mesh);
		//box2d->CreateCircleShape();
		//box2d->CreateCircleShape(7.0f, { 20.0f,0.0f });

		/*
		for (int y = -10; y < 65; y++)
		{
			for (int x = -20; x < 20; x++)
			{
				auto object = Instantiate("DynamicBox");
				object->transform.scale = { 1.0f,1.0f };
				object->transform.position = { static_cast<float>(x) * 20,static_cast<float>(y) * 10};
				object->AddComponent<Renderer>(L"asset/pic/hartR.png");
				object->AddComponent<Box2DBody>()->CreateBoxShape();
			}
		}*/

		std::vector<b2Vec2> points =
		{
			{-200,200},{200, 200} ,{ 200,-150} /*,{ 0,-150}*/ ,{-200,-150}
		};
	
		auto object = Instantiate("StaticBox");
		object->transform.position = { 0.0f,-100.0f };
		//object->transform.scale = { 100.0f,10.0f };
		//object->AddComponent<Renderer>(L"asset/pic/hartG.png");
		b2BodyDef bodyDef2 = b2DefaultBodyDef();
		//bodyDef2.type = b2_kinematicBody;
		auto box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(FILTER_03);
		box2d->CreateSegment(points);
		//object->AddComponent<Box2DBody>()->CreateBoxShape();
	}

	Vector2 mousePos;
	Vector2 cameraPos;

	int count = 0;

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_E))
		{
			DeleteObject(gameObject);
		}

		if (Input::Get().KeyTrigger(VK_L))
		{
			if (box2dbody->GetFilter() == FILTER_01)
				box2dbody->SetFilter(FILTER_03);
			else
				box2dbody->SetFilter(FILTER_01);
		}

		if (Input::Get().KeyPress(VK_D))
		{
			box2dbody->SetVelocityX(10.0f);
		}
		if (Input::Get().KeyPress(VK_A))
		{
			box2dbody->SetVelocityX(-10.0f);
		}
		if (Input::Get().KeyPress(VK_W))
		{
			box2dbody->SetVelocityY(10.0f);
		}
		if (Input::Get().KeyPress(VK_S))
		{
			box2dbody->SetVelocityY(-10.0f);
		}

		if (Input::Get().KeyPress(VK_G))
		{
			auto object = Instantiate("DynamicBox");
			object->transform.scale = { static_cast<float>(rand() % 3) + 1,static_cast<float>(rand() % 3) + 1 };
			object->transform.position = { static_cast<float>(rand() % 15) ,100 };
			auto box2d = object->AddComponent<Box2DBody>();
			box2d->SetFilter(FILTER_02);
			box2d->CreateBoxShape();
		}
		if (Input::Get().KeyPress(VK_H))
		{
			auto object = Instantiate("DynamicBox");
			object->transform.scale = { static_cast<float>(rand() % 3) + 1,static_cast<float>(rand() % 3) + 1};
			object->transform.position = { static_cast<float>(rand() % 15) ,100 };
			object->AddComponent<Box2DBody>()->CreateCircleShape();
		}
		if (Input::Get().KeyPress(VK_J))
		{
			auto object = Instantiate("DynamicBox");
			object->transform.scale = { static_cast<float>(rand() % 3) + 1,static_cast<float>(rand() % 3) + 1 };
			object->transform.position = { static_cast<float>(rand() % 15) ,100 };
			object->AddComponent<Box2DBody>()->CreateCapsuleShape();
		}

		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			SceneManager::LoadingScene<SampleScene_Box2D>();
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