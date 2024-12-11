#pragma once
#include "Thon.h"
#include "MovePlayer.h"
#include "AntiGravity.h"

class SceneGimmick : public Scene
{
public:
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
		bodyDef2.type = b2_dynamicBody;
		bodyDef2.gravityScale = 0.0f;

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
		object->AddComponent<SubWindow>("UntiGravity");
		object->AddComponent<AntiGravity>();

		object = Instantiate("StaticBox_Ground_Polygon");
		object->transform.position = { 200.0f,0.0f };
		std::vector<b2Vec2> mesh = {
			{-100,25},{100,25},{10,-50}, { -10,-50 }
		};
		bodyDef2 = b2DefaultBodyDef();
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_OBSTACLE);
		box2d->CreatePolygonShape(mesh);


		auto thon = Instantiate("thon", L"asset/pic/hartB.png");
		thon->transform.position = { 0,-150 };
		thon->AddComponent<Thon>();
	}

	void Update()
	{

	}
	
};
