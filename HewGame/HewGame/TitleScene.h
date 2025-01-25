#pragma once
#include "MovePlayer.h"
#include "Permeation.h"
#include "AntiGravity.h"
#include "uiObj.h"

// ゲーム起動時に描画されるタイトルシーン
class TitleScene : public Scene
{
private:
	int inputEN = 0;

public:

	void Load() override
	{

		auto object = Instantiate("object");
		auto text = object->AddComponent<SFText>("Title");
		auto button = object->AddComponent<Button>();
		button->SetEvent([&](){LOG("Press");});

		if(Input::Get().KeyTrigger(VK_RETURN))
		{
			switch (inputEN)
			{
			case 0:
				// サブウィンドウ
				object = Instantiate("UntiGravityWindow");
				object->transform.scale = { 30.0f,20.0f };
				auto bodyDef2 = b2DefaultBodyDef();
				bodyDef2.type = b2_kinematicBody;
				auto box2d = object->AddComponent<Box2DBody>(&bodyDef2);
				box2d->SetFilter(F_WINDOW);
				box2d->CreateBoxShape(true);
				object->AddComponent<Renderer>();
				object->AddComponent<SubWindow>("UntiGravity");
				object->AddComponent<AntiGravity>();

				inputEN = 1;
				break;
			}
			
		}
		

		
	}

	void Update() override
	{
		
	}
};


SetReflectionScene(TitleScene);
