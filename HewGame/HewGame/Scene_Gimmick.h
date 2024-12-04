#pragma once
#include "thon.h"

class SceneGimmick : public Scene
{
public:
	void Load() override
	{
		auto object = Instantiate("aaaa",L"asset/pic/hartB.png");
		object->AddComponent<Thon>();
		
	}

	void Update()
	{

	}
	
};
