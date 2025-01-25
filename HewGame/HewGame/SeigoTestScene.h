#pragma once
#include "MovePlayer.h"
#include "EnemyChaser.h"
#include "AntiGravity.h"
#include "Goal.h"
#include "MoveSubWindow.h"
#include "RockGenerator.h"
#include "CrushRock.h"
#include "RunFixedRoute.h"
#include "MoveRect.h"

class SeigoTestScene :public Scene
{
	SAFE_POINTER(GameObject, player)

	void Load()
	{
		player = Instantiate("Player", L"asset/pic/hartR.png");
		player->AddComponent<MovePlayer>();

		auto enemy = Instantiate("enemy");
		enemy->transform.position.x = 200.0f;
		enemy->AddComponent<Renderer>(L"asset/pic/hartG.png");
		auto enemyChaser = enemy->AddComponent<EnemyChaser>();
		enemyChaser->SetTarget(player);

		std::vector<b2Vec2> points =
		{
			{-800,300}, { 800, 300 } ,{ 800,-300} ,{500,-300},{500,100},{400,100},{400,-300},{-600,-300}, { -800,-100 }
		};
		auto object = Instantiate("StaticBox_Map");
		b2BodyDef bodyDef2 = b2DefaultBodyDef();
		bodyDef2.type = b2_staticBody;
		bodyDef2.gravityScale = 0.0f;

		auto box2dChain = object->AddComponent<Box2DBody>(&bodyDef2);
		box2dChain->CreateSegment(points);

		object = Instantiate("StaticBox_Obstacle");
		object->transform.position = { -200.0f,-100.0f };
		object->transform.scale = { 20.0f,10.0f };
		bodyDef2 = b2DefaultBodyDef();
		auto box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_OBSTACLE);
		box2d->CreateBoxShape();

		object = Instantiate("StaticBox_Ground_Polygon");
		object->transform.position = { 200.0f,0.0f };
		std::vector<b2Vec2> mesh = {
			{-100,25},{100,25},{10,-50}, { -10,-50 }
		};
		bodyDef2 = b2DefaultBodyDef();
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_OBSTACLE);
		box2d->CreatePolygonShape(mesh);

		auto window = Instantiate("AntiGravityWindow");
		window->transform.scale = { 30.0f,20.0f };
		auto bodyDef = b2DefaultBodyDef();
		bodyDef.type = b2_kinematicBody;
		auto box2D = window->AddComponent<Box2DBody>(&bodyDef);
		box2D->SetFilter(F_WINDOW);
		box2D->CreateBoxShape(true);
		window->AddComponent<Renderer>();
		window->AddComponent<SubWindow>("AntiGravity");
		window->AddComponent<AntiGravity>();

	}

	void Update()
	{
		
	}
};
