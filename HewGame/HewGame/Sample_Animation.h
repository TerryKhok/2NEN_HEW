#pragma once

class SampleScene_Title;

class SampleScene_Animation : public Scene
{
	SAFE_POINTER(Animator,anim)
	SAFE_POINTER(GameObject,object)

	void Load()
	{
		object = Instantiate("Character", L"asset/pic/rzqLOX.png");
		//rend = object->AddComponent<Renderer>(L"asset/pic/rzqLOX.png");
		//rend->SetTexcode(8, 9, 0, 0);
		object->transform.position.x = 100.0f;
		anim = object->AddComponent<Animator>();

		auto obj = Instantiate();
		obj->transform.scale.x = 8;
		obj->SetLayer(LAYER_UI);
		auto rend = obj->AddComponent<Renderer>();
		rend->SetColor({ 0, 0, 1, 1 });
		auto text = obj->AddComponent<SFText>("aaaaa‚ ‚„‚Ó‚Ÿ\n‚¦‚ ‚¦‚Ó‚Ÿ‚¦‚†");
		text->offset = { 0,10 };

		/*wchar_t* texPath;
		float scaleX = 0.5f;
		float scaleY = 0.5f;
		int frameX = 0;
		int frameY = 0;
		long long waitCount = 0;*/

		float scaleX = 1.0f / 8;
		float scaleY = 1.0f / 9;

		std::vector<AnimationFrame> frames =
		{
			{
			L"asset/pic/rzqLOX.png",
			scaleX,
			scaleY,
			0,
			0,
			10000000
			},
			{
			L"asset/pic/rzqLOX.png",
			scaleX,
			scaleY,
			1,
			0,
			10000000
			},
			{
			L"asset/pic/rzqLOX.png",
			scaleX,
			scaleY,
			0,
			1,
			10000000
			},
			{
			L"asset/pic/rzqLOX.png",
			scaleX,
			scaleY,
			1,
			1,
			10000000
			},
		};

		anim->AddClip("Idel", frames);
		anim->Play("Idel");
	}

	Vector2 oldMousePos;

	void Update()
	{

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