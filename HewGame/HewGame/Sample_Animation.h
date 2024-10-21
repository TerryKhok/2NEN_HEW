#pragma once

class SampleScene_Title;

class SampleScene_Animation : public Scene
{
	SAFE_POINTER(Animator,anim)
	SAFE_POINTER(GameObject,object)

	void Load()
	{
		object = Instantiate("text");
		auto rend = object->AddComponent<Renderer>();
		//object->transform.position.x += 100.0f;

		/*object = Instantiate("Character", L"asset/pic/rzqLOX.png");
		SAFE_POINTER(Renderer, rend);
		rend = object->AddComponent<Renderer>(L"asset/pic/rzqLOX.png");
		rend->SetTexcode(8, 9, 0, 0);
		anim = object->AddComponent<Animator>();*/

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
	}
};