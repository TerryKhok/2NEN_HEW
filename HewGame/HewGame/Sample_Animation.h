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
		object->transform.scale = { 50.0f,50.0f };
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

		anim->AddClip("Walk","asset/animationClip/walkDXT5.amcp");
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

		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			SceneManager::LoadScene<SampleScene_Title>();
		}


		Vector2 mousePos = Input::Get().MousePoint();
		if (Input::Get().MouseRightTrigger())
		{
			oldMousePos = mousePos;
		}

		if (Input::Get().MouseRightPress())
		{
			Vector2 dis = mousePos - oldMousePos;
			oldMousePos = mousePos;
			dis *= -1.0f;
			float rad = -Math::DegToRad(CameraManager::cameraRotation);
			Vector2 offset;
			offset.x = dis.x * cos(rad) - dis.y * sin(rad);
			offset.y = dis.x * sin(rad) + dis.y * cos(rad);
			RenderManager::renderOffset += offset / RenderManager::renderZoom;
		}

		if (Input::Get().MouseWheelDelta() > 0)
		{
			RenderManager::renderZoom += RenderManager::renderZoom / 50;
		}
		else if (Input::Get().MouseWheelDelta() < 0)
		{
			RenderManager::renderZoom -= RenderManager::renderZoom / 50;
		}
	}
};