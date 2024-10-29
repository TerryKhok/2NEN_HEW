#pragma once

class AnimationBulider_Scene : public Scene
{
	SAFE_POINTER(GameObject, imgLoadButton)
	SAFE_POINTER(GameObject, img)

	void Load();

	void Update()
	{
		//imgLoadButton->transform.position = Input::Get().MousePoint();
	}
};
