#include "MovePlayer.h"
#include "AntiGravity.h"
#include "LowGravity.h"
#include "Permeation.h"


class Scene_ProtoType :public Scene
{
	SAFE_POINTER(GameObject, player)

	void Load()
	{
		player = Instantiate("Player", L"asset/pic/hartR.png");
		player->AddComponent<MovePlayer>();

		std::vector<b2Vec2> points =
		{
			{-800,300}, { 800, 300 } ,{ 800,-300} ,{500,-300},{500,100},{400,100},{400,-300},{-600,-300}, { -800,-100 }
		};
		auto object = Instantiate("StaticBox_Map");
		b2BodyDef bodyDef2 = b2DefaultBodyDef();
		auto box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_TERRAIN);
		box2d->CreateChain(points);

		object = Instantiate("StaticBox_Obstacle");
		object->transform.position = { -200.0f,-100.0f };
		object->transform.scale = { 20.0f,10.0f };
		bodyDef2 = b2DefaultBodyDef();
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_OBSTACLE);
		box2d->CreateBoxShape();

		object = Instantiate("UntiGravityWindow");
		object->transform.scale = { 30.0f,20.0f };
		bodyDef2 = b2DefaultBodyDef();
		bodyDef2.type = b2_kinematicBody;
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_WINDOW);
		box2d->CreateBoxShape(true);
		object->AddComponent<Renderer>();
		object->AddComponent<WindowRect>("UntiGravity");
		object->AddComponent<AntiGravity>();

		object = Instantiate("LowGravityWindow");
		object->transform.scale = { 30.0f,20.0f };
		bodyDef2 = b2DefaultBodyDef();
		bodyDef2.type = b2_kinematicBody;
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_WINDOW);
		box2d->CreateBoxShape(true);
		object->AddComponent<Renderer>();
		object->AddComponent<WindowRect>("LowGravity");
		object->AddComponent<LowGravity>();

		object = Instantiate("PermeationWindow");
		object->transform.scale = { 30.0f,20.0f };
		bodyDef2 = b2DefaultBodyDef();
		bodyDef2.type = b2_kinematicBody;
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_WINDOW);
		box2d->CreateBoxShape(true);
		object->AddComponent<Renderer>();
		object->AddComponent<WindowRect>("Permeation");
		object->AddComponent<Permeation>();


		object = Instantiate("StaticBox_Ground_Polygon");
		object->transform.position = { 200.0f,0.0f };
		std::vector<b2Vec2> mesh = {
			{-100,25},{100,25},{10,-50}, { -10,-50 }
		};
		bodyDef2 = b2DefaultBodyDef();
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_OBSTACLE);
		box2d->CreatePolygonShape(mesh);
	}

	Vector2 oldMousePos;

	void Update()
	{
		if (Input::Get().KeyPress(VK_G))
		{
			auto object = Instantiate("DynamicBox");
			object->transform.scale = { static_cast<float>(rand() % 3) + 1,static_cast<float>(rand() % 3) + 1 };
			object->transform.position = { static_cast<float>(rand() % 15) ,100 };
			auto box2d = object->AddComponent<Box2DBody>();
			box2d->SetFilter(FILTER_02);
			box2d->CreateBoxShape();
		}

		if (Input::Get().KeyTrigger(VK_B))
		{
			SceneManager::LoadScene<Scene_ProtoType>();
		}

		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			SceneManager::LoadingScene<Scene_ProtoType>();
			SceneManager::ChangeScene();
		}

		/*if (Input::Get().MouseRightTrigger())
		{
			oldMousePos = Input::Get().MousePoint();
		}

		if (Input::Get().MouseRightPress())
		{
			Vector2 dis = Input::Get().MousePoint() - oldMousePos;
			oldMousePos = Input::Get().MousePoint();
			dis *= -1.0f;
			RenderManager::renderOffset += dis;
		}

		if (Input::Get().MouseWheelDelta() > 0)
		{
			RenderManager::renderZoom += 0.01f;
		}
		else if (Input::Get().MouseWheelDelta() < 0)
		{
			RenderManager::renderZoom -= 0.01f;
		}*/
	}
};