#pragma once

class Box2D_SampleScene :public Scene
{
	void Load()
	{
		auto object = Instantiate("DynamicBox");
		object->transform.scale.x = 5.0f;
		object->AddComponent<Box2DBody>()->CreateBoxShape();

		object = Instantiate("StaticBox");
		object->transform.position = { 50.0f,-200.0f,0.5f };
		b2BodyDef bodyDef2 = b2DefaultBodyDef();
		object->AddComponent<Box2DBody>(&bodyDef2)->CreateBoxShape();
	}

	void Update()
	{
		if (Input::Get().KeyTrigger(VK_SPACE))
		{
			SceneManager::LoadScene<Box2D_SampleScene>();
		}
	}
};