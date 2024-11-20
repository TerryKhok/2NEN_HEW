#pragma once

class SampleScene_Title;

class SampleScene_Animation : public Scene
{
	SAFE_POINTER(Animator,anim)
	SAFE_POINTER(GameObject,object)

	//Sound sound;

	void Load()
	{
		//sound.LoadWavFile("asset/sound/se/ƒWƒƒƒ“ƒv_1.wav");

		object = Instantiate("Character");
		//rend = object->AddComponent<Renderer>(L"asset/pic/rzqLOX.png");
		//rend->SetTexcode(8, 9, 0, 0);
		//object->transform.position.x = 100.0f;
		anim = object->AddComponent<Animator>();

		/*auto obj = Instantiate();
		obj->transform.scale.x = 8;
		auto rend = obj->AddComponent<Renderer>();
		rend->SetColor({ 0, 0, 1, 1 });
		auto text = obj->AddComponent<SFText>("aaaaa‚ ‚„‚Ó‚Ÿ\n‚¦‚ ‚¦‚Ó‚Ÿ‚¦‚†");
		text->offset = { 0,10 };*/


		float scaleX = 1.0f / 6;
		float scaleY = 1.0f / 1;

		std::vector<AnimationFrame> frames =
		{
			{
			L"asset/pic/spritesheet.png",
			scaleX,
			scaleY,
			0,
			0,
			4000000
			},
			{
			L"asset/pic/spritesheet.png",
			scaleX,
			scaleY,
			1,
			0,
			4000000
			},
			{
			L"asset/pic/spritesheet.png",
			scaleX,
			scaleY,
			2,
			0,
			4000000
			},
			{
			L"asset/pic/spritesheet.png",
			scaleX,
			scaleY,
			3,
			0,
			4000000
			},
			{
			L"asset/pic/spritesheet.png",
			scaleX,
			scaleY,
			4,
			0,
			4000000
			},
			{
			L"asset/pic/spritesheet.png",
			scaleX,
			scaleY,
			5,
			0,
			4000000
			},
		};

		anim->AddClip("Walk", frames);
		anim->Play("Walk");
	}

	Vector2 oldMousePos;

	void Update()
	{
		/*if (Input::Get().KeyTrigger(VK_SPACE))
		{
			sound.SoundPlay(0.1f, false);
		}*/

		if (Input::Get().KeyTrigger(VK_P))
		{
			static bool pause = false;
			if (pause)
			{
				anim->Resume();
				pause = false;
			}
			else
			{
				anim->Pause();
				pause = true;
			}
		}

		if (Input::Get().KeyTrigger(VK_E))
		{
			object->RemoveComponent<Animator>();
		}

		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			SceneManager::LoadScene<SampleScene_Title>();
		}

		if (Input::Get().MouseRightTrigger())
		{
			oldMousePos = Input::Get().MousePoint();
			//cameraPos = CameraManager::cameraPosition;
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
		}
	}
};