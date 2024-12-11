#pragma once
#include "MovePlayer.h"
#include "Permeation.h"
#include "AntiGravity.h"
#include "Thon.h"
#include "Rock.h"
#include "BreakScaffold.h"
#include "Fog.h"

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

		object = Instantiate("PermeationWindow");
		object->transform.scale = { 30.0f,20.0f };
		bodyDef2 = b2DefaultBodyDef();
		bodyDef2.type = b2_kinematicBody;
		box2d = object->AddComponent<Box2DBody>(&bodyDef2);
		box2d->SetFilter(F_PERWINDOW);
		box2d->CreateBoxShape({ 28.0f,18.0f }, { 0.0f,0.0f }, 0.0f, true);
		object->AddComponent<Renderer>();
		auto window = object->AddComponent<SubWindow>("Permeation");
		window->DisableDrawLayer(LAYER_FOG);
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


		auto thon = Instantiate("thon", L"asset/pic/hartB.png");
		thon->transform.position = { -250.0f,-200.0f };
		thon->transform.scale = { 5.0f,5.0f };
		thon->AddComponent<Thon>();

		auto rock = Instantiate("rock", L"asset/pic/hartG.png");
		rock->transform.position = { 250.0f,-150.0f };
		rock->transform.scale = { 15.0f,15.0 };
		rock->AddComponent<Rock>();

		auto breakScaffold = Instantiate("breakScaffold", L"asset/pic/BreakScaffold.png");
		breakScaffold->transform.position = { -420.0f,50.0f };
		breakScaffold->transform.scale = { 35.9f,29.0f };	// x: 35.0f, y: 29.0f ‚ð’´‚¦‚é‚ÆŽÀs‚Å‚«‚È‚¢
		breakScaffold->AddComponent<BreakScaffold>();


		auto fog = Instantiate("fog", L"asset/pic/fog.png");
		fog->GetComponent<Renderer>()->SetLayer(LAYER_FOG);
		fog->transform.position = { 0.0f,0.0f };
		fog->transform.scale = { 50.0f,50.0f };
		fog->AddComponent<Fog>();
	}

	void Update()
	{

	}
	
};
