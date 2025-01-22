#pragma once
#include "tSltScene.h"
#include "MovePlayer.h"
#include "Permeation.h"
#include "AntiGravity.h"
#include "ObjMH.h"
#include "ObjMW.h"

// �Q�[���N�����ɕ`�悳���^�C�g���V�[��
class TitleScene : public Scene
{
	void Load() override
	{

		auto object = Instantiate("object");
		auto text = object->AddComponent<SFText>("Title");
		auto button = object->AddComponent<Button>();
		button->SetEvent([&](){LOG("Press");});
		
		//++++++++�^�C�g���p�I�u�W�F�N�g++++++++++
		auto tObjMH = Instantiate("tObjMH", L"asset/pic/enemy.png");
		tObjMH->GetComponent<Renderer>()->SetLayer(LAYER_FOG);
		tObjMH->transform.position = { -100.0f,-100.0f };
		tObjMH->transform.scale = { 10.0f,10.0f };
		tObjMH->AddComponent<ObjMH>();

		//++++++++++++++++++++++++++++++++++++++++

		// �T�u�E�B���h�E
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

		
	}

	void Update() override
	{
		if (Input::Get().KeyTrigger(VK_RETURN)) { SceneManager::LoadScene<tSltScene>(); }
	}
};


SetReflectionScene(TitleScene);
